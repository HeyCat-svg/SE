# A SIMPLE MAZE IN HTML
## The overview of widget
There is only a widget in this website which contains a maze,two input components and a button.
     
These two input components can adjust the scale of the maze. User can input the width and height of the maze or do nothing which means maze will use the default setting of the scale of the maze.
## How to build a website(Code in html)
All the JavasCript code is from the file "maze.js". The web site just realizes the most basic function. There is actually no UI at all.
     
All the objects are crowed at the left-top site of the page.
     
The input component is set by`< input type="text" id="width" value="120">` and `< input type="text" id="height" value="80">` which set the default scale of the maze as 120*80.
     
When the button is clicked, the page will call the function called `init(width, height)` and the function will reset the maze and change the scale  of the maze.
     
## How to run the maze(Code in JavaScript)
Running the maze can be divided into several steps.
     
1. A canvas should be created to hold the maze. The size of the canvas is decided by the width and heigth entered by the user or the default setting.Its color is set as "#00FF00" as default. The canvas is created by the function `createCanvas()` in maze.js.
2. Now is time to create maze. Maze is created by DFS algorithm which means there's only one way connecting any two points in this maze. The maze is created by the function `createMaze()` in maze.js.
3. When the maze is created. User should decide the starting point and the ending point by clicking the black part in the maze. Then point clicked will change its color to yellow. Function `getCursorPos()` is responsible to this part.
4. After the starting point and the ending point is decied, the page starts to search the way from starting point to ending point using BFS algorithm too. When the program meets ramp in the searching way, it will choose the direction which leads to the shortest distance. But this strategy is sloppy because the shortest distance doesn't mean the right path. At one time, I try to sove the maze by BFS algorithm, but this strategy seems to be much more slower because it traverses more than a half of the maze. This part is completed by the founction `creatMaze()`in maze.js. 
5. Of course, all the painting work is compoleted by `drawMaze()`, the function which traverses the whole canvas from times to times, wasting much of time.
## How to improve the efficiency
The algorithm given by teacher is BFS, which is used to generate the maze and solve the maze. But the disadvantage of the algorithm is when slover chooses a wrong path, it will take solver many time to trace back to the previous ramp. Besides, `drawMaze()` also waste much time to traverse the whole canvas. So I try using BFS, which is much stable than DFS. But the efficiency doesn't change a lot. Actually, I don't know how to improve the efficiency in this case even if I searched some related information on the Internet.
