var ctx, wid, hei, cols, rows, maze, stack = [], start = {x:-1, y:-1}, end = {x:-1, y:-1}, grid = 8, padding =0, s, density=0.5;
var maxheap;
var startA = {point:start,weight:-1,length:0},endA = {point:end,weight:-1,length:-1};
var startL = start,endL = end,startT = start,endT = end;
var mazeDir,mazePointer;
var dfslength,Alength,LCAlength;
var tmp,errorPoint;
function MaxHeap(initDataArray, maxSize = 9999) {
    let arr=initDataArray || [];
    let currSize=arr.length;
 //填充heap，目前还不是一个堆
    let heap=new Array(arr.length);

    function init() {

        for(let i=0; i<currSize;i++){
            heap[i]=arr[i];
        }
        //最后一个分支节点的父节点
        let currPos=Math.floor((currSize-2)/2);
        while (currPos>=0){
			//局部自上向下下滑调整
            shif_down(currPos, currSize-1);
            //调整下一个分支节点
            currPos--;
        }

    }

    function shif_down(start,m) {
        //父节点
        let parentIndex=start,
            //左子节点
            maxChildIndex=parentIndex*2+1;
        while (maxChildIndex<=m){
            if(maxChildIndex<m && heap[maxChildIndex]<heap[maxChildIndex+1]){
                //一直指向最大关键码最大的那个子节点
                maxChildIndex=maxChildIndex+1;
            }
            if(heap[parentIndex].weight<=heap[maxChildIndex].weight){
                break;
            }else {
                //交换
                let temp=heap[parentIndex];
                heap[parentIndex]=heap[maxChildIndex];
                heap[maxChildIndex]=temp;
                parentIndex=maxChildIndex;
                maxChildIndex=maxChildIndex*2+1
            }
        }
    }
    /**
     * 插入一个数据
     *
     * @param {*} data 数据值
     * @returns {boolean} isSuccess 返回插入是否成功
     */
    this.insert = function (data) {
        heap[currSize]=data;
        shif_up(currSize);
        currSize++;
        return true;
    };
    this.isinList = function(data){
        for(var i=0;i<heap.length;++i)
            if(heap[i].point.x==data.point.x && heap[i].point.y == data.point.y)
                return true;
        return false
    }
    function shif_up(start) {
        let childIndex=start;   //当前叶节点
        let parentIndex=Math.floor((childIndex-1)/2); //父节点
        while (childIndex>0){
            //如果大就不交换
            if(heap[parentIndex].weight<=heap[childIndex].weight){
                break;
            }else {
                let temp=heap[parentIndex];
                heap[parentIndex]=heap[childIndex];
                heap[childIndex]=temp;
                childIndex=parentIndex;
                parentIndex=Math.floor((parentIndex-1)/2);
            }
        }
    }

    /**
     * @description 移除根元素，并返回根元素数据
     *
     * @returns {*} data 根元素的数据值
     */
    this.removeRoot = function () {
        if(currSize<=0)   return null;
        let maxValue=heap[0];
        heap[0]=heap[currSize-1];
        currSize--;
        shif_down(0, currSize-1);
        return maxValue;
    };

    init();

}

//0 empty rec 1: block 2:rec in searching 3:rec after serch  
function drawMaze() {
    for( var i = 0; i < cols; i++ ) {
        for( var j = 0; j < rows; j++ ) {
            /*
            switch( maze[i][j] ) {
                case 0: ctx.fillStyle = "black"; break;
                case 1: ctx.fillStyle = "gray"; break;
                case 2: ctx.fillStyle = "red"; break;
                case 3: ctx.fillStyle = "yellow"; break;
                case 4: ctx.fillStyle = "#500000"; break;
                case 8: ctx.fillStyle = "blue"; break;
            }
            ctx.fillRect( grid * i, grid * j, grid, grid  );
            ctx.strokeStyle = ctx.fillStyle;
            ctx.strokeRect( grid * i, grid * i, grid, grid  );*/
            drawBlock(i,j,maze[i][j]);
        }
    }
}
/**
 * @description draw (sx sy) block in type a 
 * @param {*} sx x in coordinate
 * @param {*} sy y in coordinate
 * @param {*} a block type
 */
function drawBlock(sx, sy, a) {
    switch( a ) {
        case 0: ctx.fillStyle = "black"; break;
        case 1: ctx.fillStyle = "gray"; break;
        case 2: ctx.fillStyle = "red"; break;
        case 3: ctx.fillStyle = "yellow"; break;
        case 4: ctx.fillStyle = "#500000"; break;
        case 8: ctx.fillStyle = "blue"; break;
    }
    ctx.fillRect( grid * sx, grid * sy, grid, grid  );
    ctx.strokeStyle = ctx.fillStyle;
    ctx.strokeRect( grid * sx+0.5, grid * sy+0.5, grid, grid  );
}

function getFNeighbours( sx, sy, a ) {
    var n = [];
    if( sx - 1 > 0 && maze[sx - 1][sy] % 8 == a ) {
        n.push( { x:sx - 1, y:sy } );
    }
    if( sx + 1 < cols - 1 && maze[sx + 1][sy] % 8 == a ) {
        n.push( { x:sx + 1, y:sy } );
    }
    if( sy - 1 > 0 && maze[sx][sy - 1] % 8 == a ) {
        n.push( { x:sx, y:sy - 1 } );
    }
    if( sy + 1 < rows - 1 && maze[sx][sy + 1] % 8 == a ) {
        n.push( { x:sx, y:sy + 1 } );
    }
    return n;
}
function startSolve()
{
    document.getElementById("btnCreateMaze").setAttribute("disabled", "disabled");
    document.getElementById("output").innerHTML = "output:<br />";
    var searchType = document.getElementById("schType").value;
    //console.log
    if(searchType == "DFS")
    {
        solveMaze1();
    }
    if(searchType == "A*")
    {
        Alength = 0;
        startA = {point:Object.create(start),weight:Math.abs(start.x-end.x)+Math.abs(start.y-end.y),length:0};
        endA = {point:Object.create(end),weight:0,length:Math.abs(start.x-end.x)+Math.abs(start.y-end.y)};
        //console.log('start A*');
        maxheap = new MaxHeap();
        mazePointer_clear();
        solveMaze_A();
    }
    if(searchType == "LCA")
    {
        LCAlength = 0;
        
        startL =Object.create(start);
        startT =Object.create(start);
        endL = Object.create(end);
        endT = Object.create(end);
        solveMaze_LCA();
    }


   

   //console.log('start LCA');
  // 


}
function solveMaze1() {
    if( start.x == end.x && start.y == end.y ) {
        for( var i = 0; i < cols; i++ ) {
            for( var j = 0; j < rows; j++ ) {
                switch( maze[i][j] ) {
                    case 2: maze[i][j] = 3; break;
                    case 4: maze[i][j] = 0; break;
                }
            }
        }
        document.getElementById("btnMazeClear").removeAttribute("disabled");
        document.getElementById("output").innerHTML += "search end";
        document.getElementById("btnCreateMaze").removeAttribute("disabled");
        drawMaze();stack = [];
        return;
    }
    var neighbours = getFNeighbours( start.x, start.y, 0 );
    if( neighbours.length ) {
        stack.push( start );
        start = neighbours[0];
        maze[start.x][start.y] = 2;
        drawBlock(start.x,start.y,2);
    } else {
        maze[start.x][start.y] = 4;
        drawBlock(start.x,start.y,4);
        start = stack.pop();
    }
 
   // drawMaze();
    requestAnimationFrame( solveMaze1 );
}

function mazePointer_clear(){
    for( var i = 0; i < cols; i++ ) {
        for( var j = 0; j < rows; j++ ) {
            mazePointer[i][j] = undefined;
        }
    }
}
function solveMaze_A(){
    if( startA.point.x == endA.point.x && startA.point.y == endA.point.y ) {

     //   console.log('length',startA.length);
      //  console.log('A* search length',Alength);
        for( var i = 0; i < cols; i++ ) {
            for( var j = 0; j < rows; j++ ) {
                switch( maze[i][j] ) {
                    case 2: maze[i][j] = 0; break;
                    case 4: maze[i][j] = 0; break;
                }
            }
        }
        while(mazePointer[endA.point.x][endA.point.y])
        {
            maze[endA.point.x][endA.point.y] = 3;
            console.log(endA.point.x,endA.point.y);
            endA.point =Object.create(mazePointer[endA.point.x][endA.point.y]);
        }
        maze[endA.point.x][endA.point.y] = 3;
        document.getElementById("btnMazeClear").removeAttribute("disabled");
        document.getElementById("output").innerHTML += "search end";
        document.getElementById("btnCreateMaze").removeAttribute("disabled");
        drawMaze();
        //console.log('start dfs');
       // dfslength = 0;
      //  maze_clear();
        //solveMaze();
        return;
    }
    var neighbours = getFNeighbours( startA.point.x, startA.point.y, 0 );
   // console.log('neighboursLength', neighbours.length );
    if( neighbours.length ) {
      //  console.log('infor');
        for(var i=0;i< neighbours.length;++i)
        {

            var nowPointA = {point: Object.create(neighbours[i]),weight:0,length:startA.length+1};
            nowPointA.weight =  nowPointA.length+Math.abs(nowPointA.point.x-endA.point.x)+Math.abs(nowPointA.point.y-endA.point.y);
           // if(maxheap.isinList(nowPointA))
             //  
            maxheap.insert(nowPointA);
            mazePointer[nowPointA.point.x][nowPointA.point.y] = {x:startA.point.x,y:startA.point.y};

        }
    }

    maze[startA.point.x][startA.point.y] = 2;
    drawBlock(startA.point.x,startA.point.y,2);
    startA = maxheap.removeRoot();
    ++Alength;
   // drawMaze();
    requestAnimationFrame( solveMaze_A );
}
function solveMaze_LCA()
{
    if( (mazeDir[startL.x][startL.y]!=undefined && maze[startL.x][startL.y ]==2 )||( mazeDir[endL.x][endL.y]!=undefined && maze[endL.x][endL.y] == 2 )) {
        var middlePoint;
        if(mazeDir[startL.x][startL.y]!=undefined && maze[startL.x][startL.y ]==2)
            middlePoint = startL;
        else
            middlePoint = endL;
        maze_clear();
        startL = Object.create(startT);
        endL = Object.create(endT);

        maze[startL.x][startL.y] = 3;
        maze[endL.x][endL.y] = 3;

        while(mazeDir[startL.x][startL.y]!=undefined && (startL.x!=middlePoint.x ||startL.y!=middlePoint.y))//
        {
            maze[startL.x][startL.y] = 3;
            startL = mazeDir[startL.x][startL.y];
        }
        while(mazeDir[endL.x][endL.y]!=undefined &&( endL.x!=middlePoint.x ||endL.y!=middlePoint.y))//mazeDir[endL.x][endL.y]!=undefined &&
        {
            maze[endL.x][endL.y] = 3;
            endL = mazeDir[endL.x][endL.y];

        }
        maze[middlePoint.x][middlePoint.y] = 3;

       // console.log('LCA search length',LCAlength);
       document.getElementById("btnMazeClear").removeAttribute("disabled");
       document.getElementById("output").innerHTML += "search end";
       document.getElementById("btnCreateMaze").removeAttribute("disabled");
        drawMaze();
        return;
    }
    var l;
    maze[startL.x][startL.y] = 2;
    drawBlock(startL.x,startL.y,2);
    maze[endL.x][endL.y] = 2;
    drawBlock(endL.x,endL.y,2);
    if(mazeDir[startL.x][startL.y]!=undefined)
    {

        startL = mazeDir[startL.x][startL.y];
       // console.log("startL",startL.x,startL.y);
        ++LCAlength;
    }
    if(mazeDir[endL.x][endL.y]!=undefined)
    {
        endL = mazeDir[endL.x][endL.y];
      //  console.log("endL",endL.x,endL.y);
        ++LCAlength;
    }
    //drawMaze();
    requestAnimationFrame( solveMaze_LCA );
}
// listen to mouseclick to start solving maze; type 8:start/end point in maze
function getCursorPos( event ) {
    var rect = this.getBoundingClientRect();
    var x = Math.floor( ( event.clientX - rect.left ) / grid / s), 
        y = Math.floor( ( event.clientY - rect.top  ) / grid / s);
    if( maze[x][y] ) return;
    if( start.x == -1 ) {
        start = { x: x, y: y };
        maze[start.x][start.y] = 8;
        drawBlock(start.x,start.y,8);
    } else {
        end = { x: x, y: y };
        maze[end.x][end.y] = 8;
        drawBlock(end.x,end.y,8);
        startSolve();
    }
}
function getNeighbours( sx, sy, a ) {
    var n = [];
    if( sx - 1 > 0 && maze[sx - 1][sy] == a && sx - 2 > 0 && maze[sx - 2][sy] == a ) {
        n.push( { x:sx - 1, y:sy } ); n.push( { x:sx - 2, y:sy } );
    }
    if( sx + 1 < cols - 1 && maze[sx + 1][sy] == a && sx + 2 < cols - 1 && maze[sx + 2][sy] == a ) {
        n.push( { x:sx + 1, y:sy } ); n.push( { x:sx + 2, y:sy } );
    }
    if( sy - 1 > 0 && maze[sx][sy - 1] == a && sy - 2 > 0 && maze[sx][sy - 2] == a ) {
        n.push( { x:sx, y:sy - 1 } ); n.push( { x:sx, y:sy - 2 } );
    }
    if( sy + 1 < rows - 1 && maze[sx][sy + 1] == a && sy + 2 < rows - 1 && maze[sx][sy + 2] == a ) {
        n.push( { x:sx, y:sy + 1 } ); n.push( { x:sx, y:sy + 2 } );
    }
    return n;
}
function createArray( c, r ) {
    var m = new Array( c );
    for( var i = 0; i < c; i++ ) {
        m[i] = new Array( r );
        for( var j = 0; j < r; j++ ) {
            m[i][j] = 1;
        }
    }
    return m;
}
/**
 * @description clear the path in the maze
 */
function maze_clear()
{
    for( var i = 0; i < cols; i++ ) {
        for( var j = 0; j < rows; j++ ) {
            if(maze[i][j]==3||maze[i][j]==2||maze[i][j]==8)
                maze[i][j]= 0;
        }
    }
    start = {x:-1,y:-1};
    drawMaze();
}
function createMaze1() {
    var neighbours = getNeighbours( start.x, start.y, 1 ), l;
    if( neighbours.length < 1 ) {
        if( stack.length < 1 ) {
            drawMaze(); stack = [];
            start.x = start.y = -1;
            document.getElementById( "canvas" ).addEventListener( "mousedown", getCursorPos, false );
            document.getElementById("btnCreateMaze").removeAttribute("disabled");

            return;
        }
        start = stack.pop();
    } else {
        var i = 2 * Math.floor( Math.random() * ( neighbours.length / 2 ) )
        l = neighbours[i]; 
        maze[l.x][l.y] = 0;
        mazeDir[l.x][l.y] = {x:start.x,y:start.y};
       // drawBlock(l.x,l.y,0);
        l = neighbours[i + 1]; 
        maze[l.x][l.y] = 0;
        mazeDir[l.x][l.y] = {x:neighbours[i].x,y:neighbours[i].y};
       // drawBlock(l.x,l.y,0);
        start = l
        stack.push( start )
    }
    drawMaze();
    requestAnimationFrame( createMaze1 );
}

function createMaze1NonAni() {

    while(true) {

        var neighbours = getNeighbours( start.x, start.y, 1 ), l;
        if( neighbours.length < 1 ) {
            if( stack.length < 1 ) {
                drawMaze(); stack = [];
                start.x = start.y = -1;
                document.getElementById( "canvas" ).addEventListener( "mousedown", getCursorPos, false );
                document.getElementById("btnCreateMaze").removeAttribute("disabled");
    
                return;
            }
            start = stack.pop();
        } else {
            var i = 2 * Math.floor( Math.random() * ( neighbours.length / 2 ) )
            l = neighbours[i]; 
            maze[l.x][l.y] = 0;
            mazeDir[l.x][l.y] = {x:start.x,y:start.y};

            l = neighbours[i + 1]; 
            maze[l.x][l.y] = 0;
            mazeDir[l.x][l.y] = {x:neighbours[i].x,y:neighbours[i].y};

            start = l
            stack.push( start )
        }    
    }
    document.getElementById("btnCreateMaze").removeAttribute("disabled");
}
function createMaze2() {

    var r = Math.random();

    maze[start.x][start.y] = r < density ? 0 : 1;
    
    drawMaze();

    if(start.x == (cols - 1) && start.y == (rows - 1)){

        document.getElementById("btnCreateMaze").removeAttribute("disabled");
        return;
    }

    start.x = start.x + 1;
    if(start.x == cols){
        start.x = 0;
        start.y = start.y + 1;
    }

    requestAnimationFrame(createMaze2);
}

function createMaze2NonAni() {

    for(var i = 0; i < cols; i++){
        for(var j = 0; j < rows; j++){
            maze[i][j] = Math.random() < density ? 0 : 1;
    
            drawBlock(i, j, maze[i][j]);
        }
    }
    document.getElementById("btnCreateMaze").removeAttribute("disabled");
}
/**  
 * @description createCanvas
 * */
function createCanvas() {
    var canvas = document.createElement( "canvas" );
    wid = document.getElementById("maze").offsetWidth - padding; 
    hei = 400;
    
    canvas.width = wid; canvas.height = 400;
    canvas.id = "canvas";
    ctx = canvas.getContext( "2d" );
    ctx.fillStyle = "gray"; ctx.fillRect( 0, 0, wid, hei );
    var div = document.getElementById("maze")
    div.appendChild( canvas ); 
}
// function onload
function init() {
    createCanvas();
}

// create the maze due to the options in html
function onCreate() {

    document.getElementById("btnCreateMaze").setAttribute("disabled", "disabled");

    wid = document.getElementById("maze").offsetWidth - padding; 
    hei = 400;

    cols = eval(document.getElementById("cols").value); 
    rows = eval(document.getElementById("rows").value);

    var mazeType = document.getElementById("sltType").value;

    if(mazeType == "Maze1") {
        cols = cols + 1 - cols % 2;
        rows = rows + 1 - rows % 2;    
    }

    maze = createArray( cols, rows );
    mazePointer = createArray( cols, rows );
    mazeDir = createArray( cols, rows );
    
    var canvas = document.getElementById("canvas");
    canvas.width = wid;
    canvas.height = hei;
    s = canvas.width / (grid * cols);
    canvas.height = s * grid * rows;

    ctx.scale(s, s);

    drawMaze();
    if(mazeType == "Maze1") {

        start.x = Math.floor( Math.random() * ( cols / 2 ) );
        start.y = Math.floor( Math.random() * ( rows / 2 ) );
        if( !( start.x & 1 ) ) start.x++; if( !( start.y & 1 ) ) start.y++;
        maze[start.x][start.y] = 0;
        mazeDir[start.x][start.y] = undefined;
        if(document.getElementById("chkAnimated").checked) {

            createMaze1();
        }
        else {

            createMaze1NonAni();
        }
    }
    else {

        density = document.getElementById("density").value / 100;
        start.x = 0;
        start.x = 0;

        if(document.getElementById("chkAnimated").checked) {

            createMaze2();
        }
        else {

            createMaze2NonAni();
        }
    }
}
/**
 * @description alter the options due to the mazetype
*/
function onSltType() {
    if(document.getElementById("sltType").value == "Maze2") {
        document.getElementById("density").removeAttribute("disabled");
        document.getElementById("schType").options.remove(2)
    }
    else {
        document.getElementById("density").setAttribute("disabled", "disabled");
        document.getElementById("schType").options.add(new Option("LCA","LCA"))
    }

}