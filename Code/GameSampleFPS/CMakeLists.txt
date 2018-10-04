## NOTE: Only projects that follow 'ENGINE_ROOT/Code/GameXXXX' conventions will be CMake-included for build.

## Add toggleable option for CMake
option(PROJECT_SampleFPS "Build SampleFPS project" ON)
if (NOT PROJECT_SampleFPS)
	return()
endif()

## Using default CPP template code
## Typically your files would all be in the same directory as this CMakeLists.txt file
set(CODE_DIR "${CRYENGINE_DIR}/Code/GameTemplates/cpp/FirstPersonShooter/Code")

#START-FILE-LIST
start_sources()
sources_platform(ALL)
add_sources("CrySampleFPS_uber_Player.cpp"
	SOURCE_GROUP "Components"
		"${CODE_DIR}/Components/Bullet.h"
		"${CODE_DIR}/Components/SpawnPoint.h"
		"${CODE_DIR}/Components/SpawnPoint.cpp"
		"${CODE_DIR}/Components/Player.h"
		"${CODE_DIR}/Components/Player.cpp"
)

add_sources("NoUberFile"
	SOURCE_GROUP "DLL"
		"${CODE_DIR}/StdAfx.h"
		"${CODE_DIR}/StdAfx.cpp"
		"${CODE_DIR}/GamePlugin.h"
		"${CODE_DIR}/GamePlugin.cpp"
)
end_sources()
#END-FILE-LIST

## You have to register the project as a module here.
## ${THIS_PROJECT} == "SampleFPS" and this'll make the XXX.dll that your .cryproject files should use.
## SOLUTION_FOLDER is unrelated to where you're storing the code. It's just where its shown in the solution explorer.
CryGameModule(SampleFPS PCH "${CODE_DIR}/StdAfx.cpp" SOLUTION_FOLDER "Projects/GameSampleFPS")

target_include_directories( ${THIS_PROJECT} PRIVATE 
	${CRYENGINE_DIR}/Code/CryEngine/CryCommon	## General library of useful stuff, base interfaces etc.
	${CRYENGINE_DIR}/Code/CryEngine/CryAction	## CryMannequin animation etc. includes
	${CRYENGINE_DIR}/Code/CryPlugins/CryDefaultEntities/Module ## DefaultEntity component/samples includes
)

if (WIN32 OR WIN64)
	target_link_libraries( ${THIS_PROJECT} PRIVATE shell32 Gdi32  )
elseif(DURANGO)
	target_link_libraries( ${THIS_PROJECT} PRIVATE ixmlhttprequest2 Ws2_32 )
	configure_durango_game(
		"app_id" "SampleFPS.app"
		"package_name" "SampleFPS"
		"display_name" "CRYENGINE Game SampleFPS"
		"publisher_name" "Crytek"
		"description" "CRYENGINE Sample FPS Project"
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

## Setup vstudio debugger command, overrides existing. The -project is relative to CryEngine root.
set_visual_studio_debugger_command(${THIS_PROJECT} "${OUTPUT_DIRECTORY}/GameLauncher.exe" "-project GameSampleFPS.cryproject" "yes")

## Dummy project for quick alternation between sandbox and launcher.
add_custom_target("fps_sandbox")
set_solution_folder("Projects/GameSampleFPS" "fps_sandbox")
set_visual_studio_debugger_command("fps_sandbox" "${OUTPUT_DIRECTORY}/Sandbox.exe" "-project GameSampleFPS.cryproject" "yes")