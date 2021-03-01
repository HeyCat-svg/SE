#include "Board.h"
#include "Cube.h"

#include <QGridLayout>


const QString Board::STANDARD_CUBES[16]  = {
        "AAEEGN", "ABBJOO", "ACHOPS", "AFFKPS",
        "AOOTTW", "CIMOTU", "DEILRX", "DELRVY",
        "DISTTY", "EEGHNW", "EEINSU", "EHRTVW",
        "EIOSST", "ELRTTY", "HIMNQU", "HLNNRZ"
};

const QString Board::BIG_BOGGLE_CUBES[25]  = {
        "AAAFRS", "AAEEEE", "AAFIRS", "ADENNN", "AEEEEM",
        "AEEGMU", "AEGMNN", "AFIRSY", "BJKQXZ", "CCNSTW",
        "CEIILT", "CEILPT", "CEIPST", "DDLNOR", "DDHNOT",
        "DHHLOR", "DHLNOR", "EIIITT", "EMOTTT", "ENSSSU",
        "FIPRSY", "GORRVW", "HIPRRY", "NOOTUW", "OOOTTU"
};

Board::Board(QWidget *parent, int size, const QString *cubeLetters) : QWidget(parent)
{
    this->size = size;
    this->cubes = new Cube*[size * size];
    this->letters = new QString[size * size];
    this->word=new bool[size*size];
    for (int i = 0; i < size * size; ++i)
        this->letters[i] = cubeLetters[i];

    shake();//打乱顺序
    /*for(int i=0;i<size*size;++i){          //make Q a useful letter
        bool flag=false;
        if(letters[i].at(0)=="Q"){
            int row=i/size;
            int col=i%size;
            for(int j=row-1;j<=row+1;++j){
                for(int k=col-1;k<=col+1;++k){
                    if(j>=0&&j<size&&k>=0&&k<size&&!(j==row&&k==col)){
                        letters[index(j,k)][0]='U';
                        flag=true;
                        break;
                    }
                }
                if(flag) break;
            }
        }
    }*/
    buildGraph();//构造邻接表

    QGridLayout *layout = new QGridLayout();
    layout->setMargin(20);
    layout->setSpacing(10);
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            this->cubes[index(i, j)] = new Cube(i,j,this);
            layout->addWidget(this->cubes[index(i, j)], i, j, Qt::AlignmentFlag::AlignCenter);
        }
    }
    setLayout(layout);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            this->cubes[index(i, j)]->setLetter(this->letters[index(i, j)].at(0));
        }
    }
    // this->setStyleSheet("background-color:grey; border: 3px solid");
}

Board::~Board()
{
    if (cubes) delete[] cubes;
    if (letters) delete[] letters;
    delete[] book;
    delete[] word;
    clearGraph();
}

void Board::shake()
{
    // Shake Cubes
    for (int i = 0; i < size * size; ++i)
        this->letters[i] = BIG_BOGGLE_CUBES[i];

    for(int i=0;i<size*size;++i){
        int swap=randEx(i,size*size-1);
        QString tmp=letters[swap];
        letters[swap]=letters[i];
        letters[i]=tmp;
    }
    for(int i=0;i<size*size;++i){
        int swap=randEx(0,5);
        QChar tmp=letters[i].at(swap);
        letters[i][swap]=letters[i].at(0);
        letters[i][0]=tmp;
    }

    for(int i=0;i<size*size;++i){          //make Q a useful letter
            bool flag=false;
            if(letters[i].at(0)=="Q"){
                int row=i/size;
                int col=i%size;
                for(int j=row-1;j<=row+1;++j){
                    for(int k=col-1;k<=col+1;++k){
                        if(j>=0&&j<size&&k>=0&&k<size&&!(j==row&&k==col)){
                            letters[index(j,k)][0]='U';
                            flag=true;
                            break;
                        }
                    }
                    if(flag) break;
                }
            }
    }
}

int Board::randEx(int min,int max){
    LARGE_INTEGER seed;
    QueryPerformanceFrequency(&seed);
    QueryPerformanceCounter(&seed);
    srand(seed.QuadPart);
    return min+rand()%(max-min+1);
}

int Board::find(QChar v)const{
    for(int i=0;i<size*size;++i)
        if(verList[i].ch==v) return i;
}

void Board::insert(int start,int end){
    verList[start].head=new edgeNode(end,verList[start].head);
}

void Board::buildGraph(){
    int col,row;
    verList=new verNode[size*size];
    book=new bool[size*size];
    for(int i=0;i<size;++i)
        book[i]=false;
    for(int i=0;i<size*size;++i)
        verList[i].ch=letters[i].at(0);
    for(int i=0;i<size*size;++i){
        row=i/size;
        col=i%size;
        for(int j=row-1;j<=row+1;++j)
            for(int k=col-1;k<=col+1;++k){
                if(j>=0&&j<size&&k>=0&&k<size&&!(j==row&&k==col))
                    insert(i,index(j,k));
            }
    }
}

void Board::clearGraph(){
    for(int i=0;i<size*size;++i){
        for(edgeNode *p=verList[i].head;p!=NULL;){
            edgeNode *tmp=p;
            p=p->next;
            delete tmp;
        }
    }
    delete[] verList;
}

void Board::searchHelper(int pos,QString str){
    if(str==NULL){flag=true;return;}
    if(pos==-1){
        for(int i=0;i<size*size;++i){
            if(verList[i].ch==str.at(0)&&book[i]==false){
                book[i]=true;
                searchHelper(i,str.mid(1));
                if(flag==true) return;
                else book[i]=false;
            }
        }
    }
    else{
        for(edgeNode *p=verList[pos].head;p!=NULL;p=p->next){
            if(verList[p->end].ch==str.at(0)&&book[p->end]==false){
                book[p->end]=true;
                searchHelper(p->end,str.mid(1));
                if(flag==true) return;
                else book[p->end]=false;
            }
        }
    }
}

bool Board::search(QString str){
    searchHelper(-1,str);
    if(flag)
        for(int i=0;i<size*size;++i)
            word[i]=book[i];
    for(int i=0;i<size*size;++i)
        book[i]=false;
    bool tmp=flag;
    flag=false;
    return tmp;
}
