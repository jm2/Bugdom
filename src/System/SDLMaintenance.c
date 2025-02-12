// SDL MAINTENANCE.C
// (C) 2020 Iliyas Jorio
// This file is part of Bugdom. https://github.com/jorio/bugdom

#include "game.h"
#include "killmacmouseacceleration.h"
#include "version.h"
#include <stdio.h>

char						gTypedAsciiKey = '\0';

static const uint32_t	kDebugTextUpdateInterval = 50;
static uint32_t			gDebugTextFrameAccumulator = 0;
static uint32_t			gDebugTextLastUpdatedAt = 0;
static char				gDebugTextBuffer[1024];

static void UpdateDebugStats(void)
{
	uint32_t ticksNow = SDL_GetTicks();
	uint32_t ticksElapsed = ticksNow - gDebugTextLastUpdatedAt;
	if (ticksElapsed >= kDebugTextUpdateInterval)
	{
		float fps = 1000 * gDebugTextFrameAccumulator / (float)ticksElapsed;
		snprintf(
				gDebugTextBuffer, sizeof(gDebugTextBuffer),
				"fps: %d\ntris: %d\nmeshes: %d+%d\ntiles: %ld/%ld%s\n\nx: %d\nz: %d\n\n%s\n\n\n\n\n\n\n\n\n\n\n\nBugdom %s\n%s @ %dx%d",
				(int)roundf(fps),
				gRenderStats.triangles,
				gRenderStats.meshesPass1,
				gRenderStats.meshesPass2,
				gSupertileBudget - gNumFreeSupertiles,
				gSupertileBudget,
				gSuperTileMemoryListExists ? "" : " (no terrain)",
				(int)gMyCoord.x,
				(int)gMyCoord.z,
				gLiquidCheat? "Liquid cheat ON": "",
				PROJECT_VERSION,
				glGetString(GL_RENDERER),
				gWindowWidth,
				gWindowHeight
		);
		QD3D_UpdateDebugTextMesh(gDebugTextBuffer);
		gDebugTextFrameAccumulator = 0;
		gDebugTextLastUpdatedAt = ticksNow;
	}
	gDebugTextFrameAccumulator++;
}

void DoSDLMaintenance(void)
{
	if (gShowDebugStats)
		UpdateDebugStats();

	// Reset these on every new frame
	gTypedAsciiKey = '\0';

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				CleanQuit();
				return;

			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_CLOSE:
						CleanQuit();
						return;

					case SDL_WINDOWEVENT_RESIZED:
						QD3D_OnWindowResized(event.window.data1, event.window.data2);
						break;

#if __APPLE__
					case SDL_WINDOWEVENT_FOCUS_LOST:
						// On Mac, always restore system mouse accel if cmd-tabbing away from the game
						RestoreMacMouseAcceleration();
						break;
						
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						// On Mac, kill mouse accel when focus is regained only if the game has captured the mouse
						if (SDL_GetRelativeMouseMode())
							KillMacMouseAcceleration();
						break;
#endif
				}
				break;

			case SDL_TEXTINPUT:
				// The text event gives us UTF-8. Use the key only if it's a printable ASCII character.
				if (event.text.text[0] >= ' ' && event.text.text[0] <= '~')
				{
					gTypedAsciiKey = event.text.text[0];
				}
				break;

			case SDL_MOUSEMOTION:
				if (!gEatMouse)
				{
					MouseSmoothing_OnMouseMotion(&event.motion);
				}
				break;

			case SDL_JOYDEVICEADDED:	 // event.jdevice.which is the joy's INDEX (not an instance id!)
				TryOpenController(false);
				break;

			case SDL_JOYDEVICEREMOVED:	// event.jdevice.which is the joy's UNIQUE INSTANCE ID (not an index!)
				OnJoystickRemoved(event.jdevice.which);
				break;
		}
	}
}
