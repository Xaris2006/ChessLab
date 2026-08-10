# Chess Lab

Chess Lab is a simple chess GUI that chess players can use to save, open and analyse chess games with also the help of a chess engine (default Stockfish). With this program you can create your own Chess Opening Book (COB) files that you can use to improve your perfomance in openings.

## Requirements
- [Visual Studio 2022](https://visualstudio.com) (not strictly required, however included setup scripts only support this)

## Getting Started
Once you've cloned, run `scripts/Setup-ChessLab.bat` to generate Visual Studio 2022 solution/project files. Once you've opened the solution, you can run the ChessLab-Board project to run the chess board app. To run the ChessLab-Lobby project you have to build the ChessLab-Board project in Dist configuration and copy its .exe file from 'bin\Dist-windows-x86_64\ChessLab-Board' directory to 'ChessLab-Lobby\ChessLabApp' directory.

There might a problem when building the project. Please check that all needed projects use the same setting on: c/c++ -> Code Generation -> Runtime Library.

### 3rd party libaries
- [implot](https://github.com/epezent/implot)
- [Walnut](https://github.com/StudioCherno/Walnut)

#### Some screenshots from the Chess Lab

![Screenshot](images/Screenshot2.png)
![Screenshot](images/Screenshot3.png)
![Screenshot](images/Screenshot4.png)
![Screenshot](images/Screenshot5.png)
![Screenshot](images/Screenshot6.png)
![Screenshot](images/Screenshot7.png)
![Screenshot](images/Screenshot8.png)
![Screenshot](images/Screenshot1.png)