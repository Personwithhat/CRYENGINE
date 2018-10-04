message(STATUS "OVERRIDE: Overriding user-set options. Trackable in OVERRIDE.cmake")

SET(OPTION_RC ON CACHE BOOL "a" FORCE)						## Needed to compile any shaders etc.
set(OPTION_RC_RELEASE ON CACHE BOOL "a" FORCE)				## Force release build of RC, no need to wait ages for shaders if in debug build.

SET(PROJECT_CRYENGINE_GAMESDK OFF CACHE BOOL "a" FORCE)		## Don't bother with this piece of junk.
SET(OPTION_UNITY_BUILD OFF CACHE BOOL "a" FORCE)			## Not caring about this or crymono for now.
SET(OPTION_CRYMONO OFF CACHE BOOL "a" FORCE)

## For main-branch build
SET(PLUGIN_GAMEPLATFORM OFF CACHE BOOL "a" FORCE)			## Doesn't detect if sdk exists automatically anymore
SET(AUDIO_SDL_MIXER OFF CACHE BOOL "a" FORCE)				## 5.5.0 available SDK's fail to build sdl_mixer
SET(AUDIO_FMOD ON CACHE BOOL "a" FORCE)						## Alternative audio, assumes sdk downloaded.
SET(OPTION_RUNTIME_CVAR_OVERRIDES ON CACHE BOOL "a" FORCE)	## Assuming you have more than 1 project in a solution, need this on.
SET(PLUGIN_SCHEMATYC OFF CACHE BOOL "a" FORCE)				## Conflicting CVar commands/etc., avoid compiling both schematyc plugins.

## For various MPFR memory testing as a just in case, turn this OFF when needed.
SET(OPTION_MEMREPLAY_USES_DETOURS ON CACHE BOOL "a" FORCE)