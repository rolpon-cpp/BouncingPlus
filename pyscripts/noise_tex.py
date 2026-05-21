import noise
import pygame

pygame.init()
pygame.display.set_mode((1,1))

tex = pygame.Surface((512, 512), pygame.SRCALPHA, 32)

for y in range(256):
    for x in range(256):
        clr = 255 + (127 * noise.pnoise2(x * 0.03, y * 0.03)/0.1)

        if clr < 240:
            clr -= 60

        clr *= 0.5

        clr = min(max(clr, 0),255)
        tex.set_at((x * 2, y * 2), (50, 255, 50, clr))

pygame.image.save(tex, "noise.png")
pygame.quit()