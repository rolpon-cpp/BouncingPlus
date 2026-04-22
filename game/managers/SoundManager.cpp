//
// Created by lalit on 11/5/2025.
//

#include "SoundManager.h"
#include "../Game.h"
#include <filesystem>
#include <iostream>
#include <raymath.h>

SoundManager::SoundManager(Game &game) {
    this->game = &game;

    MaxSoundPoolSize = 10;
    #ifdef PLATFORM_WEB
        MaxSoundPoolSize = 1;
    #endif

    std::string path = "assets/sounds";
    for (const auto & entry : std::filesystem::directory_iterator(path)) {
        std::string p = entry.path().filename().string();
        p.erase(p.end() - 4, p.end());
        if (!p.ends_with("music"))
        {
            Sound sound = LoadSound(entry.path().string().c_str());
            Sounds.insert({p, sound});
            std::vector<Sound> s = std::vector<Sound>();
            CachedAliases[p] = s;
        } else
        {
            Music m = LoadMusicStream(entry.path().string().c_str());
            Musics.insert({p, m});
        }
    }
    path = "assets/chasethemes";
    for (const auto & entry : std::filesystem::directory_iterator(path))
    {
        for (const auto & chase_theme_layer : std::filesystem::directory_iterator(entry))
        {
            std::string p = chase_theme_layer.path().filename().string();
            p = entry.path().filename().string() + "_" + p;
            p.erase(p.end() - 4, p.end());
            Music m = LoadMusicStream(chase_theme_layer.path().string().c_str());
            Musics.insert({p, m});
        }
    }
}

SoundManager::SoundManager() {
}

SoundManager::~SoundManager() {
}

void SoundManager::Clear() {
    for (auto& [name,value] : CachedAliases) {
        for (Sound& sound : value) {
            StopSound(sound);
        }
    }
    for (auto& [name,value] : Sounds) {
        StopSound(value);
    }
    for (auto& [name,value] : Musics) {
        StopMusicStream(value);
    }
    MusicTransitions.clear();
}

void SoundManager::Update() {

    std::erase_if(MusicTransitions, [&](tuple<string, float, float>& value) {
        std::string& s = std::get<0>(value);
        float& f1 = std::get<1>(value);
        float& f2 = std::get<2>(value);
        if (!Musics.count(s))
            return true;
        if (abs(f1 - f2) <= 0.05f)
        {
            if (f2 <= 0.05f)
                StopMusicStream(Musics[s]);
            f2 = f1;
            return true;
        }
        return false;
        });

    for (tuple<string, float, float>& value : MusicTransitions)
    {
        std::string& s = std::get<0>(value);
        float& f1 = std::get<1>(value);
        float& f2 = std::get<2>(value);

        f2 = Lerp(f2, f1, 6.5f * game->GetGameDeltaTime());
        SetMusicVolume(Musics[s],f2);
    }

    for (auto& [name,val]: Musics)
    {
        if (IsMusicStreamPlaying(val))
            UpdateMusicStream(val);
    }

    int i = 0;
    for (auto& [name,value] : CachedAliases) {
        std::erase_if(value, [&](Sound& sound) {
            bool SoundCondition = false;
            #ifdef PLATFORM_WEB
                SoundCondition = ((IsSoundValid(sound) && !IsSoundPlaying(sound)) || !IsSoundValid(sound));
            #else
                SoundCondition = (value.size() > MaxSoundPoolSize && ((IsSoundValid(sound) && !IsSoundPlaying(sound)) || !IsSoundValid(sound)));
            #endif

            if (SoundCondition) {
                if (IsSoundValid(sound))
                    UnloadSoundAlias(sound);
            }
            return SoundCondition;
        });
        std::string dbg_txt = "audio: " + name + ", amount: " + to_string(value.size());
        if (game->DebugDraw && value.size() > 0)
        {
            DrawText(dbg_txt.c_str(), GetRenderWidth()/2 + game->GameCamera.RaylibCamera.target.x, GetRenderHeight()/2 + game->GameCamera.RaylibCamera.target.y + (25*i), 25, GREEN);
            i++;
        }
    }

}

void SoundManager::ClearCache()
{
    for (auto& [name,value] : CachedAliases)
    {
        for (Sound& sound : value)
        {
            if (IsSoundValid(sound))
                UnloadSoundAlias(sound);
        }
        value.clear();
    }
}

bool SoundManager::IsGameMusicPlaying(std::string MusicName)
{
    if (!Musics.count(MusicName))
        return false;
    return IsMusicStreamPlaying(Musics[MusicName]);
}

void SoundManager::PlayGameMusic(std::string MusicName, bool Transition)
{
    if (GetMasterVolume() <= 0.0f)
        return;
    if (!Musics.count(MusicName))
        return;
    PlayMusicStream(Musics[MusicName]);
    if (!Transition)
        SetMusicVolume(Musics[MusicName], 1.0f);
    else
    {
        MusicTransitions.push_back(make_tuple(MusicName, 1.0f, 0.0f));
        SetMusicVolume(Musics[MusicName], 0);
    }
}

void SoundManager::StopGameMusic(std::string MusicName, bool Transition)
{
    if (!Musics.count(MusicName))
        return;
    if (!Transition)
        StopMusicStream(Musics[MusicName]);
    else
    {
        MusicTransitions.push_back(make_tuple(MusicName, 0.0f, 1.0f));
        SetMusicVolume(Musics[MusicName], 1.0f);
    }
}

void SoundManager::PlayGameSound(std::string SoundName, float SoundVolume, float SoundPitch) {
    if (GetMasterVolume() <= 0.0f)
        return;
    if (Sounds.count(SoundName) && IsSoundValid(Sounds[SoundName])) {
        for (Sound& CachedSound : CachedAliases[SoundName]) {
            if (IsSoundValid(CachedSound) && !IsSoundPlaying(CachedSound)) {
                SetSoundVolume(CachedSound, SoundVolume);
                SetSoundPitch(CachedSound, SoundPitch);
                PlaySound(CachedSound);
                return;
            }
        }

        #ifdef PLATFORM_WEB
        if (CachedAliases[SoundName].size() >= MaxSoundPoolSize)
        {
            Sound& CachedSound = CachedAliases[SoundName][0];
            StopSound(CachedSound);
            SetSoundVolume(CachedSound, SoundVolume);
            SetSoundPitch(CachedSound, SoundPitch);
            PlaySound(CachedSound);
            return;
        }
        #endif

        Sound CachedSound = LoadSoundAlias(Sounds[SoundName]);
        SetSoundVolume(CachedSound, SoundVolume);
        SetSoundPitch(CachedSound, SoundPitch);
        PlaySound(CachedSound);
        CachedAliases[SoundName].push_back(CachedSound);
    }
}

void SoundManager::Quit() {
    for (auto& [name,value] : CachedAliases) {
        for (Sound& sound : value) {
            if (IsSoundValid(sound)) {
                UnloadSoundAlias(sound);
            }
        }
        value.clear();
    }
    for (auto& [name,value] : Sounds) {
        if (IsSoundValid(value))
            UnloadSound(value);
    }
    for (auto& [name,value] : Musics) {
        if (IsMusicValid(value))
            UnloadMusicStream(value);
    }
    Sounds.clear();
    Musics.clear();
    CachedAliases.clear();
    MusicTransitions.clear();
}
