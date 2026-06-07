//
// Created by lalit on 11/5/2025.
//

#ifndef BOUNCINGPLUS_SOUNDMANAGER_H
#define BOUNCINGPLUS_SOUNDMANAGER_H
#include <raylib.h>
#include <unordered_map>
#include <string>
#include <vector>

class Game;

class SoundManager {
    public:
    Game *game;
    std::unordered_map<std::string, Sound> Sounds;
    std::unordered_map<std::string, Music> Musics;
    std::unordered_map<std::string, std::vector<Sound>> CachedAliases;
    std::vector<std::tuple<std::string,float,float>> MusicTransitions;
    int MaxSoundPoolSize;
    SoundManager(Game &game);
    SoundManager();
    ~SoundManager();
    void PlayGameSound(std::string SoundName, float SoundVolume = 1.0f, float SoundPitch = 1.0f);
    void PlayGameMusic(std::string MusicName, bool Transition = false);
    void StopGameMusic(std::string MusicName, bool Transition = false);
    bool IsGameMusicPlaying(std::string MusicName);
    void ClearCache();
    void Clear();
    void Update();
    void Quit();
};


#endif //BOUNCINGPLUS_SOUNDMANAGER_H