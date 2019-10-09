var ctx, wid, hei, cols, rows, maze, stack = [], start = {x:-1, y:-1}, end = {x:-1, y:-1}, grid = 8,maxheap;
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
        //如果当前大小等于最大容量
        // console.log('insert');
        // if(data ==undefined)
        //     console.log('insertdata:undefined');
        // if(currSize===maxSize){
        //     return false
        // }

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
     //   console.log('heaplength',currSize);
     //   console.log('child',childIndex);
     //  console.log('parent',parentIndex);
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
     * 移除根元素，并返回根元素数据
     *
     * @returns {*} data 根元素的数据值
     */
    this.removeRoot = function () {
        //console.log('removetop');


        if(currSize<=0){
            console.log('null');
            return null;

        }
        let maxValue=heap[0];
        heap[0]=heap[currSize-1];

        currSize--;
        shif_down(0, currSize-1);

/*
        console.log('heaplength',currSize);
        for(var i=0;i<heap.length;++i)
        if(heap[i]==undefined)
            console.log('undefined',i);
        console.log('length',heap.length);
*/
        return maxValue;
    };

    init();

}


function drawMaze() {
    for( var i = 0; i < cols; i++ ) {
        for( var j = 0; j < rows; j++ ) {
            switch( maze[i][j] ) {
                case 0: ctx.fillStyle = "black"; break;
                case 1: ctx.fillStyle = "green"; break;
                case 2: ctx.fillStyle = "red"; break;
                case 3: ctx.fillStyle = "yellow"; break;
                case 4: ctx.fillStyle = "#500000"; break;
            }
            ctx.fillRect( grid * i, grid * j, grid, grid  );
        }
    }
}
function getFNeighbours( sx, sy, a ) {
    var n = [];
    if( sx - 1 > 0 && maze[sx - 1][sy] == a ) {
        n.push( { x:sx - 1, y:sy } );
    }
    if( sx + 1 < cols - 1 && maze[sx + 1][sy] == a ) {
        n.push( { x:sx + 1, y:sy } );
    }
    if( sy - 1 > 0 && maze[sx][sy - 1] == a ) {
        n.push( { x:sx, y:sy - 1 } );
    }
    if( sy + 1 < rows - 1 && maze[sx][sy + 1] == a ) {
        n.push( { x:sx, y:sy + 1 } );
    }
    return n;
}
//dfs search 
function solveMaze() {
    if( start.x == end.x && start.y == end.y ) {
        console.log('dfs search length',dfslength);
        for( var i = 0; i < cols; i++ ) {
            for( var j = 0; j < rows; j++ ) {
                switch( maze[i][j] ) {
                    case 2: maze[i][j] = 3; break;
                    case 4: maze[i][j] = 0; break;
                }
            }
        }
        drawMaze();
        console.log('start LCA search');
        maze_clear();
        solveMaze_LCA();
        return;
    }
    var neighbours = getFNeighbours( start.x, start.y, 0 );
    if( neighbours.length ) {
        stack.push( start );
        start = neighbours[0];
        maze[start.x][start.y] = 2;
    } else {
        maze[start.x][start.y] = 4;
        start = stack.pop();

    }
    ++dfslength;
    drawMaze();
    requestAnimationFrame( solveMaze );
}

function mazePointer_clear(){
    for( var i = 0; i < cols; i++ ) {
        for( var j = 0; j < rows; j++ ) {
            mazePointer[i][j] = undefined;
        }
    }
}
function maze_clear()
{
    for( var i = 0; i < cols; i++ ) {
        for( var j = 0; j < rows; j++ ) {
            if(maze[i][j]==3||maze[i][j]==2)
                maze[i][j]= 0;
        }
    }
    drawMaze();
}
//A*
function solveMaze_A(){
    if( startA.point.x == endA.point.x && startA.point.y == endA.point.y ) {

        console.log('length',startA.length);
        console.log('A* search length',Alength);
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
            endA.point =Object.create(mazePointer[endA.point.x][endA.point.y]);
        }
        drawMaze();
        console.log('start dfs');
        dfslength = 0;
        maze_clear();
        solveMaze();
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
            mazePointer[nowPointA.point.x][nowPointA.point.y] = Object.create(startA.point);

        }
    }

    maze[startA.point.x][startA.point.y] = 2;
    startA = maxheap.removeRoot();
    if(startA==undefined)
        console.log('startundefined');
    ++Alength;
    drawMaze();
    requestAnimationFrame( solveMaze_A );
}
// LCA
function solveMaze_LCA()
{
    if( (mazeDir[startL.x][startL.y]!=undefined && maze[startL.x][startL.y ]==2 )||( mazeDir[endL.x][endL.y]!=undefined && maze[endL.x][endL.y] == 2 )) {
        var middlePoint;
        if(mazeDir[startL.x][startL.y]!=undefined && maze[startL.x][startL.y ]==2)
            middlePoint = startL;
        else
            middlePoint = endL;
       // console.log("middlePoint",middlePoint.x,middlePoint.y);
        maze_clear();
        startL = Object.create(startT);
        endL = Object.create(endT);
      //  console.log("middlePoint",middlePoint.x,middlePoint.y);

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

        console.log('LCA search length',LCAlength);
        drawMaze();
        return;
    }
    var l;
    maze[startL.x][startL.y] = 2;
    maze[endL.x][endL.y] = 2;
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
    drawMaze();
    requestAnimationFrame( solveMaze_LCA );
}
function getCursorPos( event ) {
    var rect = this.getBoundingClientRect();
    var x = Math.floor( ( event.clientX - rect.left ) / grid ), 
        y = Math.floor( ( event.clientY - rect.top  ) / grid );
    if( maze[x][y] ) return;
    if( start.x == -1 ) {
        start = { x: x, y: y };
    } else {
        end = { x: x, y: y };
       // maze[start.x][start.y] = 2;

        startL =Object.create(start);
        startT =Object.create(start);
        endL = Object.create(end);
        endT = Object.create(end);
        
        startA = {point:Object.create(start),weight:Math.abs(start.x-end.x)+Math.abs(start.y-end.y),length:0};
        endA = {point:Object.create(end),weight:0,length:Math.abs(start.x-end.x)+Math.abs(start.y-end.y)};

        Alength = 0;
        LCAlength = 0;
        

        mazePointer_clear(); console.log('start A*'); solveMaze_A();


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
function createMaze() {
  //  console.log('create maze start point',start.x,start.y);
  ++tmp;
    var neighbours = getNeighbours( start.x, start.y, 1 ), l;
    if( neighbours.length < 1 ) {
        if( stack.length < 1 ) {
            drawMaze(); stack = [];
          //  var tmpPoint = mazeDir[errorPoint.x][errorPoint.y];
        //    console.log("errorpoint",mazeDir[errorPoint.x][errorPoint.y]);
         //   console.log(start);
           // console.log(tmpPoint);
          //  console.log('-1-1');
            document.getElementById( "canvas" ).addEventListener( "mousedown", getCursorPos, false );
            //start  = undefined;
            start.x = -1;
            start.y = -1;
           // console.log("errorpoint",mazeDir[errorPoint.x][errorPoint.y]);
            return;
        }
        start = stack.pop();

    } else {
        var i = 2 * Math.floor( Math.random() * ( neighbours.length / 2 ) )

        l = neighbours[i]; maze[l.x][l.y] = 0;
            mazeDir[l.x][l.y] = {x:start.x,y:start.y};
        l = neighbours[i + 1]; maze[l.x][l.y] = 0;
            mazeDir[l.x][l.y] = {x:neighbours[i].x,y:neighbours[i].y};
    /*   if(tmp==2)
        {
            errorPoint = neighbours[i];
         //   console.log("errorpoint",errorPoint.x,errorPoint.y);
         //   console.log("errorpoint",mazeDir[errorPoint.x][errorPoint.y]);
        }*/
        start =  Object.create(l);
        stack.push( start );
    }

   createMaze();
 // drawMaze();
   // requestAnimationFrame(createMaze );// 
    return;
}
function createCanvas( w, h ) {
    var canvas = document.createElement( "canvas" );
    wid = w; hei = h;
    canvas.width = wid; canvas.height = hei;
    canvas.id = "canvas";
    ctx = canvas.getContext( "2d" );
    ctx.fillStyle = "black"; ctx.fillRect( 0, 0, wid, hei );
    document.body.appendChild( canvas ); 
}
function init() {
    cols = 120; rows = 80;
    maxheap = new MaxHeap();
    createCanvas( grid * cols, grid * rows );
    maze = createArray( cols, rows );
    mazePointer = createArray( cols, rows );
    mazeDir = createArray( cols, rows );
    start.x = Math.floor( Math.random() * ( cols / 2 ) );
    start.y = Math.floor( Math.random() * ( rows / 2 ) );
    if( !( start.x & 1 ) ) start.x++; if( !( start.y & 1 ) ) start.y++;
    maze[start.x][start.y] = 0;
    mazeDir[start.x][start.y] = undefined;
    console.log('maze start point',start.x,start.y);
    tmp = 0;
    createMaze();
  //  console.log("errorpoint",errorPoint.x,errorPoint.y);
 //   requestAnimationFrame( );
}
