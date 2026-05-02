import noise
import pygame

pygame.init()
pygame.display.set_mode((1,1))

tex = pygame.Surface((256, 256), pygame.SRCALPHA, 32)

for y in range(128):
    for x in range(128):
        clr = 255 + (127 * noise.pnoise2(x * 0.15, y * 0.15)/0.3)

        clr *= 0.5

        clr = min(max(clr, 0),255)
        tex.set_at((x * 2, y * 2), (50, 255, 50, clr))

pygame.image.save(tex, "noise.png")
pygame.quit()