## Add toggelable option for CMake
option(PROJECT_OffsetWindow "Build OffsetWindow project" ON)
if (NOT PROJECT_OffsetWindow)
	return()
endif()

## Using local code .-.
set(CODE_DIR ".")

#START-FILE-LIST
start_sources()
sources_platform(ALL)
add_sources("NoUberFile"
    SOURCE_GROUP "Root"
		"${CODE_DIR}/StdAfx.h"
		"${CODE_DIR}/StdAfx.cpp"
)
add_sources("OffsetWindow_uber_Core.cpp"
    SOURCE_GROUP "Plugin"
		"${CODE_DIR}/GamePlugin.h"
		"${CODE_DIR}/GamePlugin.cpp"
)
add_sources("OffsetWindow_uber_Player.cpp"
	SOURCE_GROUP "Components"
		"${CODE_DIR}/Components/Player.h"
		"${CODE_DIR}/Components/Player.cpp"
)
end_sources()
#END-FILE-LIST

CryGameModule(OffsetWindow PCH "${CODE_DIR}/StdAfx.cpp" SOLUTION_FOLDER "Projects/GameOffsetWindow")

target_include_directories( ${THIS_PROJECT} PRIVATE 
	${CODE_DIR}									## Include YOUR project into the include path!
	${CRYENGINE_DIR}/Code/CryEngine/CryCommon	## General library of useful stuff, base interfaces etc.
	${CRYENGINE_DIR}/Code/CryEngine/CryAction	## CryMannequin etc. includes
	${CRYENGINE_DIR}/Code/CryPlugins/CryDefaultEntities/Module ## DefaultEntity component includes
)

if (WIN32 OR WIN64)
	target_link_libraries( ${THIS_PROJECT} PRIVATE shell32 Gdi32  )
elseif(DURANGO)
	target_link_libraries( ${THIS_PROJECT} PRIVATE ixmlhttprequest2 Ws2_32 )
	configure_durango_game(
		"app_id" "GameOffsetWindow.app"
		"package_name" "GameOffsetWindow"
		"display_name" "CRYENGINE Game OffsetWindow"
		"publisher_name" "Crytek"
		"description" "CRYENGINE Debug project to get window spawn offset"
		"foreground_text" "light"
		"background_color" "#6495ED"
		"version" "1.0.0.0"
		"titleid" "06BFF3BF"
		"scid" "84d70100-5e91-4cf0-bcc9-7b2f06bff3bf"
		"appxmanifest" "AppxManifest.xml"
		"logo" "DurangoLogo.png"
		"small_logo" "DurangoSmallLogo.png"
		"splash_screen" "DurangoSplashScreen.png"
		"store_logo" "DurangoStoreLogo.png"
	)
endif()

## Setup vstudio debugger command, overrides existing.
set_visual_studio_debugger_command(${THIS_PROJECT} "${OUTPUT_DIRECTORY}/GameLauncher.exe" "-project Projects/${THIS_PROJECT}/Game.cryproject" "yes")

## Dummy project for quick alternation between sandbox and launcher.
add_custom_target("off_sandbox")
set_solution_folder("Projects/GameOffsetWindow" "off_sandbox")
set_visual_studio_debugger_command("off_sandbox" "${OUTPUT_DIRECTORY}/Sandbox.exe" "-project Projects/${THIS_PROJECT}/Game.cryproject" "yes")

## Two dummy projects, used to debug networking by launching more than one instance/etc.
## Ofc add debugger options here too.
add_custom_target("off_game2") 
set_solution_folder("Projects/GameOffsetWindow" "off_game2")
set_visual_studio_debugger_command("off_game2" "${OUTPUT_DIRECTORY}/GameLauncher.exe" "-project Projects/${THIS_PROJECT}/Game2.cryproject" "yes")

add_custom_target("off_server")
set_solution_folder("Projects/GameOffsetWindow" "off_server")
set_visual_studio_debugger_command("off_server" "${OUTPUT_DIRECTORY}/Game_Server.exe" "-project Projects/${THIS_PROJECT}/Server.cryproject" "yes")
