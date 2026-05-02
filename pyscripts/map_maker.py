import json
import time

import pygame
import os
import random
import csv

tilemap_width = 0
tilemap_height = 0
tilemap_path = ""

rs = random.randint(32,64)

def thread1():
    global tilemap_width, tilemap_height, tilemap_path

    import tkinter

    m = tkinter.Tk()

    w = tkinter.Label(m, text="Bouncing Plus Map Maker")
    w.pack()

    frame = tkinter.Frame()
    frame.pack()

    g = tkinter.Label(frame, text="Map Path (leave blank if you are making a new level)")
    g.grid(row=0, column=0)

    h = tkinter.Label(frame, text="Map Width (leave blank if you are modding a level)")
    h.grid(row=1, column=0)

    k = tkinter.Label(frame, text="Map Height (leave blank if you are modding a level)")
    k.grid(row=2, column=0)

    e1 = tkinter.Entry(frame)
    e2 = tkinter.Entry(frame)
    e3 = tkinter.Entry(frame)
    e1.grid(row=0, column=1)
    e2.grid(row=1, column=1)
    e3.grid(row=2, column=1)

    def set_val():
        global tilemap_width, tilemap_height, tilemap_path

        try:
            tilemap_width = int(e2.get())
        except ValueError:
            tilemap_width = rs

        try:
            tilemap_height = int(e3.get())
        except ValueError:
            tilemap_height = rs

        try:
            tilemap_path = str(e1.get())
        except ValueError:
            tilemap_path = ""

        m.destroy()

    button = tkinter.Button(m, text='Done', width=25, command=set_val)
    button.pack()

    m.mainloop()

thread1()

assets_folder_path = "assets\\"
if not os.path.exists(assets_folder_path):
    assets_folder_path = "..\\assets\\"

tile_size = 36

pygame.init()

screen = pygame.display.set_mode((1280,720))
clock = pygame.time.Clock()

running = True

current_mode = 1
current_tile = 0
tilemap = []
placed_entities = []
selected_entities = []
camera = [0, 0]

fnt = pygame.font.SysFont("Consolas",50)

if len(tilemap_path) < 1:
    for y in range(tilemap_height):
        row = []
        for x in range(tilemap_width):
            row.append(-1)
        tilemap.append(row)
else:
    with open(os.path.join(assets_folder_path+"maps\\"+tilemap_path+"\\entities.csv")) as ef:
        content = ef.read()
        for line in content.splitlines():
            if line == "Type,X,Y,Width,Height,Health,Armor,Speed,Weapon":
                continue
            vals = line.split(",")
            try:
                placed_entities.append(
                    [
                        int(vals[0]),
                        float(vals[1]),
                        float(vals[2]),
                        float(vals[3]),
                        float(vals[4]),
                        float(vals[5]),
                        float(vals[6]),
                        float(vals[7]),
                        str(vals[8])
                    ]
                )
            except:
                print("failed to read entity")
    with open(os.path.join(assets_folder_path+"maps\\"+tilemap_path+"\\map_data.csv")) as data:
        data = csv.reader(data, delimiter=',')
        for raw_row in data:
            row = list(raw_row)
            new_row = []
            for id in row:
                if id != "":
                    n_id = int(id)
                    new_row.append(n_id)
            tilemap.append(new_row)
    tilemap_width = len(tilemap[0])
    tilemap_height = len(tilemap)

images_to_id = {
    0: "bouncy_wall.png",
    1: "delete_wall.png",
    2: "enemy.png",
    3: "sword.png",
    4: "shotgun.png",
    5: "player.png",
    6: "spawner.png",
    7: "warning.png",
    8: "upgrade_core.png",
    9: "boss1_img.png",
    10: "turret.png",
    11: "enemy_wall_tile.png"
}

for key in images_to_id.keys():
    try:
        images_to_id[key] = pygame.image.load("map_builder_assets\\" + images_to_id[key]).convert_alpha()
    except Exception as e:
        print(e)

last_mouse_pos = [0, 0]

undo_strokes = []
strokes = []
stroke = []

ui_state_txt = "NO MODE ACTIVE"

last_e_state = False
last_c_state = False
last_drag_state = False
last_ctrlz_state = False
last_ctrly_state = False

last_s_state = False

hovering_over_selection_box = False

tile_mode = True

entity_selection_rect = pygame.Rect(-1, -1, 0, 0)
entity_selection_rect_points = [[0,0],[0,0],[0,0],[0,0]]
selecting_entities = False
selecting_entities_before = False

entity_num = -1
entity_x = -1
entity_y = -1
entity_w = -1
entity_h = -1
entity_health = -1
entity_armor = -1
entity_speed = -1
entity_weapon = ""

def place_entity(x,y):
    global entity_num
    global entity_x
    global entity_y
    global entity_w
    global entity_h
    global entity_health
    global entity_armor
    global entity_speed
    global entity_weapon

    entity_x = x
    entity_y = y

    import tkinter

    m = tkinter.Tk()

    w = tkinter.Label(m, text="New/Edit Entity")
    w.pack()

    frame = tkinter.Frame()
    frame.pack()

    g = tkinter.Label(frame, text="type")
    g.grid(row=0, column=0)

    h = tkinter.Label(frame, text="w")
    h.grid(row=1, column=0)

    k = tkinter.Label(frame, text="h")
    k.grid(row=2, column=0)

    h1 = tkinter.Label(frame, text="health")
    h1.grid(row=3, column=0)

    k1 = tkinter.Label(frame, text="armor")
    k1.grid(row=4, column=0)

    k2 = tkinter.Label(frame, text="speed")
    k2.grid(row=5, column=0)

    k3 = tkinter.Label(frame, text="weapon")
    k3.grid(row=6, column=0)

    e1 = tkinter.Entry(frame)
    e2 = tkinter.Entry(frame)
    e3 = tkinter.Entry(frame)
    e1.grid(row=0, column=1)
    e2.grid(row=1, column=1)
    e3.grid(row=2, column=1)

    e4 = tkinter.Entry(frame)
    e5 = tkinter.Entry(frame)
    e6 = tkinter.Entry(frame)
    e4.grid(row=3, column=1)
    e5.grid(row=4, column=1)
    e6.grid(row=5, column=1)

    e7 = tkinter.Entry(frame)
    e7.grid(row=6, column=1)

    def set_val():
        global entity_num
        global entity_x
        global x
        global y
        global entity_y
        global entity_w
        global entity_h
        global entity_health
        global entity_armor
        global entity_speed
        global entity_weapon

        try:
            entity_num = int(e1.get())
        except ValueError:
            pass

        try:
            entity_w = float(e2.get())
        except ValueError:
            pass

        try:
            entity_h = float(e3.get())
        except ValueError:
            pass

        try:
            entity_health = float(e4.get())
        except ValueError:
            pass

        try:
            entity_armor = float(e5.get())
        except ValueError:
            pass

        try:
            entity_speed = float(e6.get())
        except ValueError:
            pass

        try:
            entity_weapon = str(e7.get())
        except ValueError:
            pass

        m.destroy()

    button = tkinter.Button(m, text='Done', width=25, command=set_val)
    button.pack()

    m.mainloop()

    if entity_num != -1:
        new_e = [entity_num, entity_x, entity_y, entity_w, entity_h, entity_health, entity_armor, entity_speed, entity_weapon]
        for e in placed_entities:
            er = pygame.Rect(e[1]/2 - e[3]/4,e[2]/2 - e[4]/4,e[3]/2,e[4]/2)
            if er.collidepoint(world_mouse_pos[0],world_mouse_pos[1]):
                placed_entities.remove(e)
                break
        placed_entities.append(new_e)
        entity_num = -1
        entity_x = -1
        entity_y = -1
        entity_w = -1
        entity_h = -1
        entity_health = -1
        entity_armor = -1
        entity_speed = -1
        entity_weapon = ""

cool = time.time()

while running:

    dt = clock.tick(240) / 1000

    mouse_pos = pygame.mouse.get_pos()
    rel_x = last_mouse_pos[0] - mouse_pos[0]
    rel_y = last_mouse_pos[1] - mouse_pos[1]
    last_mouse_pos = mouse_pos

    screen.fill((100,100,100))

    mouse_buttons = pygame.mouse.get_pressed()
    keys = pygame.key.get_pressed()

    world_mouse_pos = list(mouse_pos)
    world_mouse_pos[0] += camera[0]
    world_mouse_pos[1] += camera[1]

    world_mouse_tile_pos = [max(min(int(world_mouse_pos[0] / tile_size), tilemap_width-1),0),max(min(int(world_mouse_pos[1] / tile_size), tilemap_height-1),0)]

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.MOUSEBUTTONUP and event.button == 1 and not tile_mode and keys[pygame.K_r]:
            place_entity(world_mouse_pos[0]*2,world_mouse_pos[1]*2)

    if keys[pygame.K_k] and time.time() - cool >= 0.2:
        tile_mode = not tile_mode
        cool = time.time()

    if tile_mode:
        if current_mode == 1:
            ui_state_txt = "TILE PLACING"
        elif current_mode == 2:
            ui_state_txt = "TILE REMOVAL"

        if mouse_buttons[2]:
            current_mode = 0

        if keys[pygame.K_c] and not last_c_state:
            for y in range(tilemap_height):
                for x in range(tilemap_width):
                    if tilemap[y][x] != -1:
                        stroke.append([tilemap[y][x], -1, [x,y]])
                        tilemap[y][x] = -1
            strokes.append(stroke.copy())
            stroke.clear()
        last_c_state = keys[pygame.K_c]

        if keys[pygame.K_e] and keys[pygame.K_e] != last_e_state:
            current_mode = 2 if current_mode == 1 else 1
        last_e_state = keys[pygame.K_e]

        drag_state = mouse_buttons[0] and (not hovering_over_selection_box) and current_mode > 0
        if drag_state:
            if tilemap[world_mouse_tile_pos[1]][world_mouse_tile_pos[0]] != (current_tile if current_mode == 1 else -1):
                stroke.append([tilemap[world_mouse_tile_pos[1]][world_mouse_tile_pos[0]], current_tile if current_mode == 1 else -1, world_mouse_tile_pos])
            tilemap[world_mouse_tile_pos[1]][world_mouse_tile_pos[0]] = current_tile if current_mode == 1 else -1
        elif last_drag_state:
            strokes.append(stroke.copy())
            stroke.clear()
        last_drag_state = drag_state

        ctrlz_state = keys[pygame.K_z] and keys[pygame.K_LCTRL]
        if ctrlz_state and len(strokes) > 0 and not last_ctrlz_state:
            latest_stroke = strokes[len(strokes)-1]
            for t in latest_stroke:
                og = t[0]
                new = t[1]
                coord = t[2]
                tilemap[coord[1]][coord[0]] = og
            strokes.remove(latest_stroke)
            undo_strokes.append(latest_stroke)
        last_ctrlz_state = ctrlz_state

        ctrly_state = keys[pygame.K_y] and keys[pygame.K_LCTRL]
        if ctrly_state and len(undo_strokes) > 0 and not last_ctrly_state:
            latest_stroke = undo_strokes[len(undo_strokes) - 1]
            for t in latest_stroke:
                og = t[0]
                new = t[1]
                coord = t[2]
                tilemap[coord[1]][coord[0]] = new
            undo_strokes.remove(latest_stroke)
            strokes.append(latest_stroke)
        last_ctrly_state = ctrly_state
    else:
        ui_state_txt= "ENTITY MODE"
        c = False
        for e in placed_entities:
            er = pygame.Rect(e[1]/2 - e[3]/4,e[2]/2 - e[4]/4,e[3]/2,e[4]/2)
            if er.collidepoint(world_mouse_pos[0],world_mouse_pos[1]):
                c=True
                ui_state_txt = "ENTITY EDIT"
                surf=pygame.Surface((e[3],e[4]),pygame.SRCALPHA,32)
                surf.fill((0,0,255,150))
                screen.blit(surf,(e[1]/2-e[4]/2-camera[0],e[2]/2-e[3]/2-camera[1]))
                break

        if not c:
            if keys[pygame.K_r]:
                ui_state_txt = "ENTITY ADD"
                surf=pygame.Surface((36,36),pygame.SRCALPHA,32)
                surf.fill((50,50,255,150))
                screen.blit(surf,(world_mouse_pos[0]-18-camera[0],world_mouse_pos[1]-18-camera[1]))

    if mouse_buttons[1]:
        camera[0] += rel_x
        camera[1] += rel_y

    for y in range(tilemap_height):
        for x in range(tilemap_width):
            if tilemap[y][x] != -1 and images_to_id.get(tilemap[y][x]) != None:
                tex = images_to_id[tilemap[y][x]].copy()
                if tex.get_size() != (36, 36):
                    tex = pygame.transform.scale(tex, (36, 36))
                xwp = x * tile_size
                ywp = y * tile_size
                if tilemap[y][x] == 2:
                    tex = pygame.transform.scale(tex, (18, 18))
                    xwp += 9
                    ywp += 9

                tex.set_alpha(50 if not tile_mode else 255)
                screen.blit(tex, (xwp - camera[0], ywp - camera[1]))

    for e in placed_entities:
        x = e[1] / 2
        y = e[2] / 2
        w = e[3]
        h = e[4]
        x -= w/4
        y -= h/4
        styyy = pygame.transform.scale(images_to_id[2],(w/2,h/2))
        styyy.set_alpha(50 if tile_mode else 255)
        if e in selected_entities:
            surf=pygame.Surface(styyy.get_size(),pygame.SRCALPHA,32)
            surf.fill((0,0,255,150))
            styyy.blit(surf,(0,0))
        screen.blit(styyy,(x - camera[0],y - camera[1]))

    for x in range(tilemap_width+1):
        pygame.draw.line(screen, (255,255,255), [x * tile_size - camera[0], - camera[1]],[x * tile_size - camera[0], tilemap_height*tile_size - camera[1]], width=1)
    for y in range(tilemap_height+1):
        pygame.draw.line(screen, (255,255,255), [ - camera[0],y* tile_size - camera[1]],[tilemap_width*tile_size - camera[0], y * tile_size - camera[1]], width=1)

    if current_mode > 0:
        pygame.draw.rect(screen, (255, 255 if current_mode == 1 else 0, 255), (50, screen.get_height() - 100, 50, 50))

    surf = pygame.Surface((144,360),pygame.SRCALPHA,32)
    surf.fill((0, 0, 0, 128))
    rect = surf.get_rect()
    rect.x = 50
    rect.y = 50

    hovering_over_selection_box=rect.collidepoint(mouse_pos[0],mouse_pos[1])
    for y in range(10):
        for x in range(4):
            i = y * 4 + x
            if i in images_to_id:
                tex = images_to_id[i]
                if tex.get_size() != (36, 36):
                    tex = pygame.transform.scale(tex, (36, 36))
                tex_rect = tex.get_rect()
                tex_rect.x = (x * 36) + rect.x
                tex_rect.y = (y * 36) + rect.y

                if current_tile == i:
                    pygame.draw.rect(screen, (50, 255, 50), tex_rect)

                if tex_rect.collidepoint(mouse_pos[0],mouse_pos[1]):
                    pygame.draw.rect(screen, (255, 255, 255), tex_rect)
                    if mouse_buttons[0]:
                        current_tile = i
                        current_mode = 1

                surf.blit(tex, (x * 36, y * 36))

    screen.blit(surf,rect)

    ee_2 = True
    for e in selected_entities:
        er = pygame.Rect(e[1]/2 - e[3]/4,e[2]/2 - e[4]/4,e[3]/2,e[4]/2)
        if er.collidepoint(world_mouse_pos[0],world_mouse_pos[1]):
            ee_2=False
            break

    selecting_entities = not tile_mode and not keys[pygame.K_r] and mouse_buttons[0] and ee_2
    if selecting_entities and not selecting_entities_before and entity_selection_rect.collidepoint(world_mouse_pos[0],world_mouse_pos[1]):
        selecting_entities = False

    if selecting_entities and not selecting_entities_before:
        entity_selection_rect_points[0] = world_mouse_pos
        entity_selection_rect = pygame.Rect(
            min(entity_selection_rect_points[0][0], min(entity_selection_rect_points[1][0], min(entity_selection_rect_points[2][0], entity_selection_rect_points[3][0]))),
            min(entity_selection_rect_points[0][1], min(entity_selection_rect_points[1][1], min(entity_selection_rect_points[2][1], entity_selection_rect_points[3][1]))),
            max(entity_selection_rect_points[0][0], max(entity_selection_rect_points[1][0], max(entity_selection_rect_points[2][0], entity_selection_rect_points[3][0]))),
            max(entity_selection_rect_points[0][1], max(entity_selection_rect_points[1][1], max(entity_selection_rect_points[2][1], entity_selection_rect_points[3][1])))
        )
        entity_selection_rect.w -= entity_selection_rect.x
        entity_selection_rect.h -= entity_selection_rect.y
    elif selecting_entities:
        entity_selection_rect_points[1] = world_mouse_pos
        entity_selection_rect_points[2] = [entity_selection_rect_points[0][0] + (world_mouse_pos[0]-entity_selection_rect_points[0][0]),entity_selection_rect_points[0][1]]
        entity_selection_rect_points[3] = [entity_selection_rect_points[0][0],world_mouse_pos[1]]
        entity_selection_rect = pygame.Rect(
            min(entity_selection_rect_points[0][0], min(entity_selection_rect_points[1][0], min(entity_selection_rect_points[2][0], entity_selection_rect_points[3][0]))),
            min(entity_selection_rect_points[0][1], min(entity_selection_rect_points[1][1], min(entity_selection_rect_points[2][1], entity_selection_rect_points[3][1]))),
            max(entity_selection_rect_points[0][0], max(entity_selection_rect_points[1][0], max(entity_selection_rect_points[2][0], entity_selection_rect_points[3][0]))),
            max(entity_selection_rect_points[0][1], max(entity_selection_rect_points[1][1], max(entity_selection_rect_points[2][1], entity_selection_rect_points[3][1])))
        )
        entity_selection_rect.w -= entity_selection_rect.x
        entity_selection_rect.h -= entity_selection_rect.y

    if tile_mode:
        entity_selection_rect = pygame.Rect(-1,-1,0,0)
    elif entity_selection_rect.w > 0 and entity_selection_rect.h > 0:
        r = (entity_selection_rect.x -camera[0],
             entity_selection_rect.y - camera[1],
             entity_selection_rect.w,entity_selection_rect.h)
        surf = pygame.Surface(entity_selection_rect.size,pygame.SRCALPHA,32)
        surf.fill((100,100,255,128))
        screen.blit(surf,(r[0],r[1]))

    for e in placed_entities:
        er = pygame.Rect(e[1]/2 - e[3]/4,e[2]/2 - e[4]/4,e[3]/2,e[4]/2)
        if er.collidepoint(world_mouse_pos[0],world_mouse_pos[1]) and entity_selection_rect.w * entity_selection_rect.h <= 2200 and e not in selected_entities:
            selected_entities.append(e)

    for e in placed_entities:
        if e not in selected_entities and entity_selection_rect.collidepoint(e[1]/2, e[2]/2):
            selected_entities.append(e)
        #elif e in selected_entities and not entity_selection_rect.collidepoint(e[1]/2, e[2]/2):
        #    selected_entities.remove(e)
    if not selecting_entities and len(selected_entities) <= 0:
        entity_selection_rect = pygame.Rect(-1,-1,0,0)

    if not selecting_entities and mouse_buttons[0]:
        for e in selected_entities:
            e[1] -= rel_x * 2
            e[2] -= rel_y * 2

    if keys[pygame.K_DELETE]:
        for e in selected_entities:
            placed_entities.remove(e)
        selected_entities.clear()

    if not tile_mode and mouse_buttons[2]:
        selected_entities.clear()
        selecting_entities = False
        entity_selection_rect = pygame.Rect(-1,-1,0,0)

    selecting_entities_before = selecting_entities

    if keys[pygame.K_s] and not last_s_state:
        pygame.image.save(screen, "screenshot" + str(clock.get_time()) + ".png")
    last_s_state = keys[pygame.K_s]

    img = fnt.render(ui_state_txt,True,(0,0,0))
    screen.blit(img,(screen.get_width()-img.get_width(), 0))

    pygame.display.update()

pygame.quit()

write_path = ""
description_lvl = "empty"
gamemode = "default"
win_cond = "kill_all_enemies"
health = 100
speed = 350
powerup = ""
inventory_1 = ""
inventory_2 = ""
inventory_3 = ""
ffire = False
n = ""

def thread2():
    global write_path, description_lvl, gamemode, win_cond, health, speed, powerup, inventory_1, inventory_2, inventory_3, n, ffire

    import tkinter

    m = tkinter.Tk()

    w = tkinter.Label(m, text="Bouncing Plus Map Maker")
    w.pack()

    frame = tkinter.Frame()
    frame.pack()

    g = tkinter.Label(frame, text="Map Name")
    g.grid(row=0, column=0)

    h = tkinter.Label(frame, text="Description")
    h.grid(row=1, column=0)

    k = tkinter.Label(frame, text="Game mode")
    k.grid(row=2, column=0)

    k2 = tkinter.Label(frame, text="Win condition")
    k2.grid(row=3, column=0)

    k3 = tkinter.Label(frame, text="Health")
    k3.grid(row=4, column=0)

    k4 = tkinter.Label(frame, text="Speed")
    k4.grid(row=5, column=0)

    k5 = tkinter.Label(frame, text="Powerup")
    k5.grid(row=6, column=0)

    k6 = tkinter.Label(frame, text="Inventory 1")
    k6.grid(row=7, column=0)

    k7 = tkinter.Label(frame, text="Inventory 2")
    k7.grid(row=8, column=0)

    k8 = tkinter.Label(frame, text="Inventory 3")
    k8.grid(row=9, column=0)

    k9 = tkinter.Label(frame, text="Friendly Fire")
    k9.grid(row=10, column=0)

    e1 = tkinter.Entry(frame)
    e2 = tkinter.Entry(frame)
    e3 = tkinter.Entry(frame)
    e1.grid(row=0, column=1)
    e2.grid(row=1, column=1)
    e3.grid(row=2, column=1)

    e4 = tkinter.Entry(frame)
    e5 = tkinter.Entry(frame)
    e6 = tkinter.Entry(frame)
    e4.grid(row=3, column=1)
    e5.grid(row=4, column=1)
    e6.grid(row=5, column=1)

    e7 = tkinter.Entry(frame)
    e8 = tkinter.Entry(frame)
    e9 = tkinter.Entry(frame)
    e7.grid(row=6, column=1)
    e8.grid(row=7, column=1)
    e9.grid(row=8, column=1)

    e10 = tkinter.Entry(frame)
    e10.grid(row=9, column=1)

    e11 = tkinter.Entry(frame)
    e11.grid(row=10, column=1)

    def set_val():
        global write_path, description_lvl, gamemode, win_cond, health, speed, powerup, inventory_1, inventory_2, inventory_3, tilemap_path, n, ffire

        try:
            write_path = str(e1.get())
            n = str(e1.get())
        except ValueError:
            pass

        if len(tilemap_path) < 1:
            try:
                description_lvl = str(e2.get())
            except ValueError:
                pass

            try:
                gamemode = str(e3.get())
            except ValueError:
                pass

            try:
                win_cond = str(e4.get())
            except ValueError:
                pass

            try:
                health = float(e5.get())
            except ValueError:
                pass

            try:
                speed = float(e6.get())
            except ValueError:
                pass

            try:
                powerup = str(e7.get())
            except ValueError:
                pass

            try:
                inventory_1 = str(e8.get())
            except ValueError:
                pass

            try:
                inventory_2 = str(e9.get())
            except ValueError:
                pass

            try:
                inventory_3 = str(e10.get())
            except ValueError:
                pass

            try:
                ffire = bool(e11.get())
            except ValueError:
                pass

        m.destroy()

    button = tkinter.Button(m, text='Done', width=25, command=set_val)
    button.pack()

    m.mainloop()

write_path = assets_folder_path + "maps\\" + (write_path if len(write_path) > 0 and len(tilemap_path) < 1 else tilemap_path)

if len(write_path) > 0 and len(tilemap_path) < 1:
    thread2()
    write_path = assets_folder_path + "maps\\" + (write_path if len(write_path) > 0 and len(tilemap_path) < 1 else tilemap_path)
    if not os.path.exists(write_path):
        os.mkdir(write_path)
        f = open(assets_folder_path+"maps\\LevelOrder.txt", 'a')
        f.write("\n"+n)
        f.close()
    json_data = {
      "description": description_lvl,
      "difficulty": 3,
        "music": "danger",
        "player": {
        "weapon_health_gain_buff": 1,
        "starting_health": health,
        "starting_speed": speed,
        "inventory": [inventory_1, inventory_2, inventory_3],
        "powerup": powerup,
        "dash_base_power": 2500,
        "dash_power_multiplier": 1,
        "dash_base_cooldown": 1.1,
        "dash_concentration_boost": 1,
        "dodge_cooldown": 3.0,
        "dodge_iframe_time": 1.0,
        "dash_iframe_time": 0.1,
        "dash_frozen_multiplier": 0.5
      },
      "game": {
        "timer": -1,
        "win": win_cond,
        "mode": gamemode,
          "friendly_fire": ffire,
        "banned_spawn_weapons": ["Default Gun","Player Gun"]
      },
      "enemy_weapons": {
        "shotgun": "Enemy Shotgun",
        "sword": "Enemy Sword"
      }
    }

    f=open(write_path + "\\metadata.json", 'w')
    json.dump(json_data, f)
    f.close()

f = open(write_path + "\\map_data.csv", 'w')
for y in range(tilemap_height):
    line = ""
    for x in range(tilemap_width):
        id = tilemap[y][x]
        line += str(id)+("," if not (y == tilemap_height - 1 and x == tilemap_width - 1) else "")
    f.write(line+("\n" if y != tilemap_height - 1 else ""))
f.close()

f = open(write_path + "\\entities.csv", 'w')
f.write("Type,X,Y,Width,Height,Health,Armor,Speed,Weapon")
for e in placed_entities:
    f.write("\n")
    for ei in e:
        f.write(str(ei) + ",")
f.close()