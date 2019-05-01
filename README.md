## Notes:
This is guaranteed to build out of the box!  
Please ping me on discord if you have any suggestions/questions.
	
This branch significantly simplifies the startup process for coding in Cryengine by improving SDK and CMake management and development. You only need `Code/GameXXXX/CMakeLists.txt` and `.cryproject` files to integrate any additional project(s) with Cryengine.
	
The engine is basically the latest official release, *Cryengine 5.5.2*.

Just two edits to CryEngine itself, backwards compatible:  
- Improved comment syntax on XXX.cfg & .cryproject files  
- (Renderer) Hacky fix to strange view bug

## Installation:
This install guide assumes a Windows system, and pretends you have nothing installed.

If you don't see it, you don't need it. e.g. CMake is not needed, comes with CE.

*Only* exception's are C++ Distributables. I did my tests with C++2013 32/64-bit and 2017 64bit.
	
1. **Download VStudio Community 2017 64-bit.**
   - Optionally, get version [15.6.7](https://docs.microsoft.com/en-us/visualstudio/productinfo/installing-an-earlier-release-of-vs2017) as it's the one I'm currently using
   - Direct download link: [https://aka.ms/eac464](https://aka.ms/eac464)

2. **During VStudio configuration:**
   1. Select "Desktop Development with C++"
   2. Find "MFC and ATL support (x86 and x64)", location depends on VStudio version.
      - Windows SDK 10.0.16299.0 **or** 15063 required. 

3. **Double-click cry_cmake.exe**: In the drop down, Select VStudio 2017 -> Click Generate Solution. 
   - This process automatically downloads and unpacks most SDKs.
   - Ignore the perforce-plugin warning, since CryTek uses perforce internally.
   - Once GUI pops up click 'Open Project' to launch the VisualStudio sln file.

4. **Optional: Download additional SDKs**

   - **FbxSDK**: images.autodesk.com/adsk/files/fbx20161_fbxsdk_vs2015_win0.exe:
     1. Copy and rename "Program Files/Autodesk/FBX/FBX SDK/2016.1" to "Code/SDKs/FbxSdk"
     2. The path should be e.g. %ENGINEROOT%/Code/SDKs/FbxSdk/include/moreStuffetc.

   - **FMOD**: [https://www.fmod.com/download](https://www.fmod.com/download)
     1. Get the api and plugin folders.
     2. Copy "C:\Program Files (x86)\FMOD SoundSystem\FMOD Studio API Windows\XXXX" to "%ENGINEROOT%/Code/SDKs/Audio/fmod/windows/XXX"

5. **Optional: Generate again if you added SDK's** 
		
6. **Build the solution in Debug or Profile.** Obviously debug will be slower.

7. **Right click on your project In Solution Explorer (e.g. SampleFPS) and select 'Set as Startup Project':**
   - This will launch your SampleFPS project by default when you press F5/play.
   
8. **Run InstallEngine.bat, to register local engine version.**
   - It takes a bit for the icons to update on Cryengine files but you can right-click the .cryproject files and launch stuff normally.
   - *This step doesn't depend on or affect anything for Installation.* Engine-version is only used when you launch a project. So no need to re-run this every time you update CMake.
   
9. **Launch the project :)**

## Install DEBUG:
If you have little-to-no virtual memory allocated, this error will pop up during the build:
> Virtual memory could not be created for PCH.
> Compiler limit reached internal heap limit.
				
1. Go to "Adjust the appearance and performance of Windows"
2. Click advanced -> Click "Change" under virtual memory
3. Make it system-managed for the drive that's lacking it, I have ~2432MB atm.
		
## Working with PyInstall:
download\_sdks, cry\_cmake, cryselect and cryrun are python scripts wrapped into a .exe by PyInstall.  
If you want to work with them you have to do a bit of setup.
	
1. Get Python 3.5 installed in C:\Python35:
[https://www.python.org/ftp/python/3.5.4/python-3.5.4-amd64.exe](https://www.python.org/ftp/python/3.5.4/python-3.5.4-amd64.exe)
		
2. As admin-CLI. pypiwin32 is for CrySelect, make sure to re-open shell (if you using that) to get paths right setup.
    ```
    pip install PyInstaller pypiwin32
    ```

3. To run python script in CLI, run wrappers:  
    ```
    python Tools/CMake/cry_cmake/wrapper.py  
    python Tools/CMake/download_sdks/wrapper.py  
    python Tools/CryVersionSelector/cryrun_exe.py  
    python Tools/CryVersionSelector/cryselect_exe.py
    ```

4. To run PyInstall from CLI, generates and copies .exe:  
    ```
    call Tools/CMake/cry_cmake/setup xxx  
    call Tools/CMake/download_sdks/setup xxx  
    call Tools/CryVersionSelector/setup xxx
    ```
		
Make sure wrapper's include modules you add for non-wrapper use.
To activate debug mode for .exe, make sure ```debug=True``` and ```console=True``` in the XXX.spec file before PyInstalling.
		
To run .exe from CLI in case you want debug the output in the same terminal.:

```
Tools\CMake\cry_cmake\cry_cmake.exe  
Tools\CMake\download_sdks\download_sdks.exe  
Tools\CMake\CryVersionSelector\cryrun.exe  
Tools\CMake\CryVersionSelector\cryselect.exe
```
		
You can check out my SDK/cry\_cmake improvement commits for sample edits.
## General file info, more is in the respective file comments:

1. **CrySelect:**   
Updates CE-related icons and file types. This is what your right click on any .cryproject file triggers.  
 Cryselect can register engine versions, run .cryprojects directly, and more.
2. **editor.cfg:** 	Config file for Sandbox-only CVars (Console Variables) and the like.
3. **system.cfg:** 	Default Cryengine settings, .cryproject files override these.
4. **XX.cryproject:**  Your project file, its location is the project's root. It handles CVars, the target engine version and more.
5. **cryengine.cryengine:**   
Defines a custom engine version that your projects can use. You have to register this to use it.    
You can register cryengine.cryengine with InstallEngine.bat or by right-clicking it file and selecting 'Register'.
6. **cry_cmake.exe:** Configures CMake for your target VStudio version then generates solution etc.
7. **Tools/CMake/download_sdks.exe:** Downloads SDKs. Generating the solution triggers this automatically.
8. **Tools/CryVersionSelector/Utility/InstallEngine.bat:** Installs the github version of CrySelect and registers your custom Engine version.
9. **CMakeLists.txt files:**   
These files are how you use CMake to build and generate everything you have.  
Cryengine Engine building is located in Tools/CMake/XXX.cmake files.
10. **Tools/CMake/OVERRIDE.cmake**: Use this to configure permanent CMake options and have them be traceable.
	
Ignore everything WAF related, CMake is the new solution builder.

PR/Commit rules that I (try to) follow are here: 
 
- [https://docs.cryengine.com/display/CEPROG/Pull+Request+Submission](https://docs.cryengine.com/display/CEPROG/Pull+Request+Submission)
- [https://docs.cryengine.com/display/CEPROG/Commit+description+rules](https://docs.cryengine.com/display/CEPROG/Commit+description+rules)

## Annoyances:

1. **Personwithhat is messy:**  
	   In order to clean up my commits, I frequently rebase my branches. You may find that a simple git pull won't cut it.  
	   You can do merges or simply rebase everything you have, e.g. `git rebase -i origin/release_fixed`  
	   If you're worried just make a backup via `git checkout -b BAKUP` and do your tests there.

2. **Release-branch RC is messy:**
   - When building a solution, you'll always get "13", even with no changes cuz of PCH issues on RC.
   - As "!B Release branch RC changes shader case" shows, RC also has messy case issues.
    - Main branch RC has neither of these issues.

## Recommended 3-d party apps:
	
1. **AstroGrep** ([http://astrogrep.sourceforge.net/](http://astrogrep.sourceforge.net/)):   
The best windows-based file search tool, helps a lot when looking for certain lines in the engine/CMake/etc.

2. **BandiZip** ([https://www.bandisoft.com/bandizip/](https://www.bandisoft.com/bandizip/)):   
My preferred zip-manager, clean interface and runs well. Handles most types too.

3. **Notepad++** ([https://notepad-plus-plus.org/](https://notepad-plus-plus.org/)):   
Great text editor, lots of options.
	
4. **Github Desktop** ([https://desktop.github.com/](https://desktop.github.com/)):   
Great for quick commit overviews, anything complex has an option to open in CLI, and so on.
	
5. **GitExtensions** ([https://github.com/gitextensions/gitextensions/releases](https://github.com/gitextensions/gitextensions/releases)):  
	For when you need more info, an easier way to clean large files, and for the better rebase editor.