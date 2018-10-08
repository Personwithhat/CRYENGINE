# NOTES:
I recommend that you install and use AstroGrep for CryEngine source/etc. work.

>
	In progress/TODO:
		1) Update Readme's to be neater + any TODO's as below.
		2) Reworking this whole shebangle to be neater/read better.

<br />

# CryEngine Setup & Configuration

### Prepare engine files:	
>
	1) Run download\_sdks.exe Note: See .py for URL that it pulls its SDK zip from.

	2) Run InstallEngine.bat to register your "custom engine version" and location.
		Sample one's called "CRYENGINE 5.5-personal"
		The name + engine windows SDK/etc. dependancies are set in "cryengine.cryengine"
	
	3) Add windows environmental variables for ease-of-use.
		
		Control Panel -> System -> Advanced -> Environmental vars
			CryEngineRoot	  = C:\Program Files (x86)\Crytek\CRYENGINE Launcher\Crytek\CRYENGINE_5.5
			GitCryEngineRoot  = C:\Github\5.5-GITHUB-BRANCH
			(etc.)
			
### Configure your custom project:
4) You're going to need quite a few files. See:
>		
		GameSampleFPS/
		Code/GameSampleFPS

5) You can use the sample editor.cfg and system.cfg, or your own.
			Make sure you have this line in editor.cfg:
>	
	ed_useDevManager = 0					

### Build and configure solution:	
	6) Run cry_cmake, select x64 bit MSVS2017. Configure -> Generate -> Open project solution. 
	   Default options should be gud, see Tools/CMake/REVOKE_OPTIONS.cmake for more details.
			
	7) Switch to VStudio.
	   Right click RC project -> properties -> configuration manager -> Under any configurations make RC build under release.
	   Otherwise you're going to have a REALLY slow compilation process.
	   
	8) In VStudio, build your solution as normal.
			Your game project should 'prolly be in "Debug"
			Engine should (preferably) be built in "Profile", "Debug" slows it to a crawl. Up to you!
			
		To speed up a ton of VStudio stuff, allow a few resources through windows defender checks.  PERSONAL TODO: Add screenshots etc.!
		Otherwise if you do ctrl+F or build, etc. it'll freeze up more and take forever. 

	9) In VStudio, right click on your game project + 'Select as Startup Project'. Then update properties.
		Under 'Debugging':
			Command  as: $(GitCryEngineroot)\bin\win_x64\GameLauncher.exe (or Sandbox.exe)
			Argument as: -project XXX.cryproject (Your CryProject name)
					e.g. -project GameSampleFPS.cryproject		
		
	Done! 
	Now ya can just work on the code and what not as usual.
			
### Additional configuration for networking 
	The following setup will let you launch a Server instance and 2 Client instances with 1 button press in VStudio.
	Great for network debugging, can't believe nobody has done this yet.
	All 3 instances will use the same code revision etc.
	Via a specific CVar you can change the window spawn position (and stop dragging them all over the place)

	1) We need 2 bogus projects for VStudio to launch, you'll need these lines in your "Module/CMakelists.txt":
		### Two dummy projects, used to spin up several instances of the game.
		### Debug options/startup-project options aren't settable by CMake without large workarounds/mess.
		add_custom_target("Game_Instance2") 
		add_custom_target("Game_Server")

	2) Duplicate game launcher files, so that the taskbar doesn't collapse both instances into one taskbar icon.
		In CE_ROOT/bin/win_x64/ copy these, keep in same location:
			GameLauncher.exe => GameLauncher2.exe (or whatever name you want)
			GameLauncher.ilk => GameLauncher2.ilk
			GameLauncher.pbd => GameLauncher2.pbd
			
		PERSONAL TODO: In case someone builds GameLauncher changes, would need to be re-copied. Have to set up a 'duplicate' app creation when built.
					
	3) Create additional .cryproject files for instance-specific vars. You can view Game_Instance2.cryproject as a sample.

	4) After building the solution, the two new dummy projects should've popped up. Edit their properties to set up debugging etc.!
		For Game_Instance2:
			$(GitCryEngineroot)\bin\win_x64\GameLauncher2.exe
			-project Game_Instance2.cryproject

		For Game_Server:
			$(GitCryEngineroot)\bin\win_x64\Game_Server.exe
			-project GameMyFPS.cryproject
		
	6) Next, set the entire "Solution" as the "Startup Project". Edit its properties, and configure the "Startup Project" _property_.
	   In the multi-project startup option, check-mark your original project as well as the Game_Instance2 and Game_Server dummies. 
		
	That's it! You can just modify the project settings if you want more launchers/less etc.
	Breakpoints are hit when ANY of the projects hit that spot. Debug Location toolbar shows the currently interrupted process.