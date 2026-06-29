// ============================================================
//  Deadline 冲刺器 — 期末大作业模拟器
//  OOP 大作业 · 规则怪谈 · 4天极限冲刺
//  编译: g++ -std=c++17 -O2 deadline_rush.cpp -lgdi32 -mwindows -o rush.exe
// ============================================================

#include "easyx.h"
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <cstdio>
#include <cstring>

// ==================== 常量 ====================
enum { W=1280, H=800, MAP_W=800, MAP_H=800, PANEL_X=MAP_W, TILE_SZ=40, COLS=20, ROWS=20 };

// 明亮配色
const COLORREF C_BG    = RGB(248,244,235);
const COLORREF C_FLOOR1= RGB(235,225,210);
const COLORREF C_FLOOR2= RGB(225,215,200);
const COLORREF C_WALL  = RGB(180,170,155);
const COLORREF C_DOOR  = RGB(160,140,110);
const COLORREF C_PANEL_BG=RGB(252,248,238);
const COLORREF C_PANEL_BD=RGB(200,190,175);
const COLORREF C_TEXT  = RGB(60,50,40);
const COLORREF C_DIM   = RGB(140,130,120);
const COLORREF C_GOLD  = RGB(210,150,40);
const COLORREF C_GREEN = RGB(60,170,60);
const COLORREF C_RED   = RGB(200,60,60);
const COLORREF C_BLUE  = RGB(50,120,200);
const COLORREF C_ORANGE= RGB(220,140,40);
const COLORREF C_PURPLE= RGB(150,70,200);
const COLORREF C_PLAYER= RGB(80,140,200);
const COLORREF C_PDIR  = RGB(255,200,80);
const COLORREF C_BTN   = RGB(242,236,225);
const COLORREF C_BTN_H = RGB(230,220,200);
const COLORREF C_DIA_BG= RGB(252,248,240);

// 4 时段
enum TimePhase { MORNING=0, DAYTIME=1, EVENING=2, NIGHT=3 };
const wchar_t* timeNames[]={L"☀️ 早上",L"📖 白天",L"🌆 傍晚",L"🌙 深夜"};
const wchar_t* timeShort[]={L"早上",L"白天",L"傍晚",L"深夜"};
const int TIME_EFF[]={15,12,10,8}; // 写代码效率

// ==================== 工具函数 ====================
int ri(int lo,int hi){
    static std::mt19937 r{(unsigned)time(nullptr)};
    return std::uniform_int_distribution<>(lo,hi)(r);
}
int clamp(int v,int lo,int hi){ return std::max(lo,std::min(v,hi)); }
bool inRect(int mx,int my,int x,int y,int w,int h){
    return mx>=x&&mx<=x+w&&my>=y&&my<=y+h;
}

// ==================== 地图 ====================
const int dormMap[ROWS][COLS]={
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1},
    {1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// ==================== 可交互物体 · 多态继承 ====================
class MapObj {
public:
    int x,y,w,h; COLORREF clr; const wchar_t *icon,*label; bool solid;
    MapObj(int x_,int y_,int w_,int h_,COLORREF c,const wchar_t* ic,const wchar_t* lb,bool s)
        :x(x_),y(y_),w(w_),h(h_),clr(c),icon(ic),label(lb),solid(s){}
    virtual void onInteract(class Game& g)=0; // 纯虚函数 — 多态交互
    virtual ~MapObj(){}
};
// 7 个派生类 — 每个物体重写自己的交互行为
class DeskObj   :public MapObj{public: using MapObj::MapObj; void onInteract(Game& g)override;};
class BedObj    :public MapObj{public: using MapObj::MapObj; void onInteract(Game& g)override;};
class MateBObj  :public MapObj{public: using MapObj::MapObj; void onInteract(Game& g)override;};
class MateCObj  :public MapObj{public: using MapObj::MapObj; void onInteract(Game& g)override;};
class MateDObj  :public MapObj{public: using MapObj::MapObj; void onInteract(Game& g)override;};
class DoorObj   :public MapObj{public: using MapObj::MapObj; void onInteract(Game& g)override;};
class CoffeeObj :public MapObj{public: using MapObj::MapObj; void onInteract(Game& g)override;};
// 7 个具体物体实例
DeskObj   oDesk  (4,8, 3,2, RGB(70,90,120),  L"💻",L"电脑桌",true);
BedObj    oBed   (2,15,3,2, RGB(100,130,110),L"🛏️",L"床",   true);
MateBObj  oMatB  (15,2,3,2, RGB(200,140,120),L"👤",L"老卷B",true);
MateCObj  oMatC  (15,15,3,2,RGB(180,160,140),L"👤",L"阿瓜C",true);
MateDObj  oMatD  (2,2, 3,2, RGB(140,180,150),L"👤",L"老摸D",true);
DoorObj   oDoor  (9,18,4,1, C_DOOR,           L"🚪",L"走廊", false);
CoffeeObj oCoffee(1,9, 2,1, RGB(160,120,90), L"☕",L"咖啡机",false);
MapObj* objs[]={&oDesk,&oBed,&oMatB,&oMatC,&oMatD,&oDoor,&oCoffee};
const int objCount=7;

bool isWall(int tx,int ty){
    if(tx<0||tx>=COLS||ty<0||ty>=ROWS) return true;
    return dormMap[ty][tx]==1;
}
bool isDoorCell(int tx,int ty){ return !isWall(tx,ty)&&dormMap[ty][tx]==2; }
bool solidAt(int tx,int ty){
    for(int i=0;i<objCount;i++)
        if(objs[i]->solid&&tx>=objs[i]->x&&tx<objs[i]->x+objs[i]->w
           &&ty>=objs[i]->y&&ty<objs[i]->y+objs[i]->h) return true;
    return false;
}
bool canWalk(int tx,int ty){ return !isWall(tx,ty)&&!solidAt(tx,ty); }
bool nearObj(const MapObj* o,int tx,int ty){
    return tx>=o->x-1&&tx<=o->x+o->w&&ty>=o->y-1&&ty<=o->y+o->h;
}
MapObj* findNearObj(int tx,int ty){
    for(int i=0;i<objCount;i++)
        if(nearObj(objs[i],tx,ty)) return objs[i];
    return nullptr;
}

// ==================== A* 寻路 ====================
struct AStarNode{
    int x,y,g,h,f; AStarNode* parent;
    AStarNode(int x_,int y_,int g_,int h_,AStarNode* p_)
        :x(x_),y(y_),g(g_),h(h_),f(g_+h_),parent(p_){}
};
bool findPath(int sx,int sy,int tx,int ty,std::vector<std::pair<int,int>>& result){
    result.clear();
    if(!canWalk(tx,ty)) return false;
    if(sx==tx&&sy==ty) return false;
    std::vector<AStarNode*> open,closed;
    open.push_back(new AStarNode(sx,sy,0,abs(sx-tx)+abs(sy-ty),nullptr));
    int dirs[8][2]={{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
    while(!open.empty()){
        auto it=open.begin();
        for(auto i=open.begin();i!=open.end();i++) if((*i)->f<(*it)->f) it=i;
        AStarNode* curr=*it; open.erase(it); closed.push_back(curr);
        if(curr->x==tx&&curr->y==ty){
            while(curr){ result.push_back({curr->x,curr->y}); curr=curr->parent; }
            std::reverse(result.begin(),result.end());
            for(auto n:open) delete n; for(auto n:closed) delete n;
            return true;
        }
        for(int d=0;d<8;d++){
            int nx=curr->x+dirs[d][0], ny=curr->y+dirs[d][1];
            if(!canWalk(nx,ny)) continue;
            if(d==1||d==3||d==5||d==7)
                if(!canWalk(curr->x+dirs[d][0],curr->y)||!canWalk(curr->x,curr->y+dirs[d][1])) continue;
            bool inOpen=false,inClosed=false; AStarNode* existing=nullptr;
            for(auto n:open){if(n->x==nx&&n->y==ny){inOpen=true;existing=n;break;}}
            if(!inOpen) for(auto n:closed){if(n->x==nx&&n->y==ny){inClosed=true;break;}}
            if(inClosed) continue;
            int newG=curr->g+10;
            if(!inOpen) open.push_back(new AStarNode(nx,ny,newG,abs(nx-tx)+abs(ny-ty),curr));
            else if(newG<existing->g){ existing->g=newG; existing->f=newG+existing->h; existing->parent=curr; }
        }
    }
    for(auto n:open) delete n; for(auto n:closed) delete n;
    return false;
}

// ==================== Game 类 ====================
class Game {
public:
    // —— 核心数值（只有2个）——
    int progress=0;   // 作业完成度 0~100
    int anxiety=30;   // 焦虑 0~100

    // —— 时间 ——
    int day=1;
    TimePhase phase=MORNING;
    int screen=0; // 0=标题, 1=游戏中, 2=结局
    int endingId=0;
    int tick=0;

    // —— 玩家位置 ——
    int tx=6, ty=10;
    int px=6*TILE_SZ+TILE_SZ/2, py=10*TILE_SZ+TILE_SZ/2;
    int dir=0;
    bool moving=false;
    int moveDirX=0, moveDirY=0, moveProgress=0;
    int baseSpeed=4, runSpeed=8;
    bool isRunning=false;
    int targetX=-1, targetY=-1;
    std::vector<std::pair<int,int>> path;

    // —— 每日追踪 ——
    int codeCount=0;        // 今天写了几次代码
    int coffeeCount=0;
    bool deskDone=false;    // 今天写过代码
    bool talkedToB=false, talkedToC=false, talkedToD=false;
    bool dayIntroShown=false;
    bool hasMVP=false;      // D 教了「先跑通再优化」
    bool deskOpened=false;  // 打开过IDE了吗
    int bAskCount=0;        // B 被问的次数

    // —— 对话 ——
    bool showDia=false, showChoices=false;
    std::wstring diaTitle, diaText;
    std::vector<std::wstring> choices;
    int hoverSel=-1, hoverCloseBtn=0;
    int pendingCodeEff=0; // 写代码时存储的进度值
    std::vector<std::wstring> eventLog;

    // —— 规则相关 ——
    bool warnedRule2=false; // 连续写代码警告

    // ======== 工具 ========
    void addLog(const wchar_t* msg){
        eventLog.push_back(msg);
        while(eventLog.size()>6) eventLog.erase(eventLog.begin());
    }
    void clamp(){ progress=::clamp(progress,0,100); anxiety=::clamp(anxiety,0,100); }

    void pushDia(const wchar_t* t,const wchar_t* d,std::vector<std::wstring> c={}){
        diaTitle=t; diaText=d; choices=c;
        showDia=true; showChoices=!c.empty();
        hoverSel=-1; hoverCloseBtn=0;
    }

    void advancePhase(){
        int p = (int)phase;
        if(p==3){ // 深夜后 → 下一天
            day++; phase=MORNING;
            codeCount=0; coffeeCount=0;
            deskDone=false; talkedToB=talkedToC=talkedToD=false;
            dayIntroShown=false;
            warnedRule2=false;
            if(day>4) return;
        } else phase=(TimePhase)(p+1);
        clamp();
    }

    // ======== 结局判定 ========
    void checkEnding(){
        if(day>4){
            if(anxiety>=90) endingId=2;       // 代码屎山
            else if(progress>=60) endingId=0;  // 通关
            else if(progress>=30) endingId=1;  // 飘过
            else endingId=3;                   // 摆了
            screen=2;
        }
    }

    // ======== 交互：电脑桌 ========
    void doDesk(){
        if(!deskOpened){
            deskOpened=true;
            pushDia(L"💻 电脑桌", L"你终于打开了 IDE。\n\n空白的编辑器。闪烁的光标。\n你知道该写什么吗？\n\n你盯着屏幕看了很久。",{L"开始写"});
            deskDone=true;
            return;
        }
        // 规则2：连写太多次
        if(codeCount>=3){
            progress-=2; anxiety+=10;
            addLog(L"⚠️ 过度疲劳！进度-2"); clamp();
            pushDia(L"💻 电脑桌",L"你已经连续写了太多次。\n手在抖，眼睛在跳。\n屏幕上的代码开始像天书。\n\n📜 那张泛黄的纸上有句话：\n\"连续写代码不能超过两次——\n 第三次你只是在制造 bug。\"\n\n进度 -2（你写的全是错的）",{L"该休息了"});
            return;
        }
        if(codeCount==2&&!warnedRule2){
            warnedRule2=true;
            addLog(L"⚠️ 已连续写2次"); clamp();
        }

        int eff=TIME_EFF[phase];
        if(hasMVP) eff+=3; // D 的 buff
        codeCount++; deskDone=true;
        progress+=eff; anxiety+=3;
        addLog(L"💻 写代码 +进度"); clamp();

        // 深夜特殊选项
        if(phase==NIGHT&&codeCount==1){
            pushDia(L"💻 深夜写代码",L"夜深了。只有键盘声。\n你打开昨天的代码——\n有些地方你不太满意。\n\n1. 📝 写新功能（+进度，正常）\n2. 🔄 重构昨天的代码（⚠️ 规则1：深夜别重构！）",{L"写新功能",L"重构旧代码"});
            pendingCodeEff=eff;
            return;
        }
        wchar_t buf[128];
        swprintf(buf,128,L"你写了 %d 行代码。\n至少 %d 行能跑。\n焦虑 +3（每次写代码都这样）",eff,eff-2);
        pushDia(L"💻 写代码",buf,{L"继续"});
    }

    // ======== 交互：床 ========
    void doBed(){
        // 规则4：Day4 不能睡
        if(day>=4){
            pushDia(L"🛏️ 床",L"📜 那张纸的最后一条：\n\n\"Deadline 前 24 小时不能睡觉。\n 睡过去的结局是确定的——\n 你醒来的那一刻，\n 已经错过了交稿。\"\n\n你站在床边。\n你敢躺下去吗？",{L"不睡了，去交稿",L"太累了…还是睡吧"});
            return;
        }
        if(phase<EVENING){
            pushDia(L"🛏️ 床",L"现在还早。\n虽然你很困，但你心里清楚：\n现在睡了你明天会更慌。\n至少撑到傍晚吧。",{L"好吧"});
            return;
        }
        wchar_t buf[256];
        swprintf(buf,256,L"躺在床上了。\n今天结束了。\n\n做了什么：%s\n进度：%d / 100\n焦虑：%d / 100\n\n闭眼。明天还要继续。",deskDone?L"写了点代码":L"基本在摸鱼",progress,anxiety);
        anxiety-=5;
        pushDia(L"🛏️ 睡觉",buf,{L"晚安"});
        // 睡觉→下一天
        day++; phase=MORNING;
        codeCount=0; coffeeCount=0;
        deskDone=false; talkedToB=talkedToC=talkedToD=false;
        dayIntroShown=false; warnedRule2=false;
        clamp(); checkEnding();
    }

    // ======== 交互：老卷 B ========
    void talkToB(){
        if(talkedToB){
            pushDia(L"👤 老卷 B",L"今天已经聊过了。\nB 在改他的实验报告——\n他已经写了 12 个类了。\n你不想再打扰他。",{L"溜了"});
            return;
        }
        talkedToB=true; bAskCount++;

        if(day==1&&bAskCount==1){
            progress+=5; addLog(L"📈 知道了作业要求 +5进度"); clamp();
            pushDia(L"👤 老卷 B",L"B 放下书：\n\"你才刚开始？作业要写一个管理系统，\n至少 5 个类，要有继承和多态。\"\n\n你赶紧记下来。\n\"题目自拟。不过 10 个人里 8 个写图书管理。\n剩下 2 个写学生管理。\"\n\n你沉默了。\n\"你自己想一个吧。\"\n\n+5 进度（你至少知道要做什么了）",{L"谢谢大佬"});
        }
        else if(day==2||(day==1&&bAskCount==2)){
            // 规则5 触发点
            if(progress<15){
                std::wstring s1=L"B 看了你一眼：\n\"你写到哪了？\"\n\n你现在进度是 "+std::to_wstring(progress)+L"%。\n说实话还是吹牛？\n\n1. 😅「快写完了」\n2. 😰「才写了 "+std::to_wstring(progress)+L"%」";
                pushDia(L"👤 老卷 B",s1.c_str(),{L"快写完了(吹牛)",L"如实回答"});
                return;
            }
            progress+=5; addLog(L"📈 B 指导 +5进度"); clamp();
            pushDia(L"👤 老卷 B",L"B 看了一眼你的代码：\n\"你的继承关系画反了。\"\n\n他花了十分钟帮你理清逻辑。\n\"先搞清楚哪个是基类，哪个是派生类。\n这是 OOP 最基本的东西。\"\n\n+5 进度",{L"受教了"});
        }
        else if(day==3){
            progress+=3; addLog(L"📈 B 帮你看代码 +3进度"); clamp();
            { std::wstring s2=L"B 叹了口气：\n\"你已经问了 "+std::to_wstring(bAskCount)+L" 次了。\n最后一次——看好了。\"\n\n他帮你看了一遍代码结构。\n\n+3 进度"; pushDia(L"👤 老卷 B",s2.c_str(),{L"最后一次"}); }
        }
        else {
            pushDia(L"👤 老卷 B",L"B 写完了。\n他在排实验报告的格式。\n\"加油吧。你能写完的。\"\n\n他难得说了句鼓励的话。",{L"冲了"});
        }
    }

    // ======== 交互：阿瓜 C ========
    void talkToC(){
        if(talkedToC){
            pushDia(L"👤 阿瓜 C",L"C 在刷视频。\n今天已经聊过了。\n他抬头笑了一下，继续刷。",{L"溜了"});
            return;
        }
        talkedToC=true;
        anxiety-=8; clamp(); addLog(L"😌 阿瓜的安慰 -8焦虑");
        const wchar_t* quips[]={
            L"C 放下手机：\n\"交了就有分。\n这是我学长说的，他大三了。\"\n\n-8 焦虑",L"C：\"复制粘贴也是编程。\n怎么就不是了？\n你复制过来，改一下参数——\n这叫代码复用。\"\n\n-8 焦虑",L"C：\"面向对象？\n对象都没有，怎么写面向对象。\n——我的意思是，别太认真。\"\n\n-8 焦虑",L"C 看着你焦虑的脸：\n\"你焦虑的时候想想我。\n我更菜，但我心态好。\"\n\n-8 焦虑",L"C：\"你知道什么是多态吗？\n同一个 Deadline，不同的崩溃方式。\n这就是多态。\"\n\n-8 焦虑",L"C 放下手机认真地说：\n\"百度也是学习。\n你百度一个问题，学到一个答案——\n这不就是学习吗？\"\n\n-8 焦虑"};
        pushDia(L"👤 阿瓜 C",quips[ri(0,5)],{L"谢谢"});
    }

    // ======== 交互：老摸 D ========
    void talkToD(){
        if(talkedToD&&hasMVP){
            pushDia(L"👤 老摸 D",L"D 在假装学习。\n今天已经聊过了。\n他桌上那个 IDE 开了一周了。",{L"溜了"});
            return;
        }
        talkedToD=true;

        if(day==1){
            pushDia(L"👤 老摸 D",L"D 刚从床上爬起来：\n\"你也还没开始？太好了。\n我以为全宿舍就我一个。\"\n\n他打开 IDE——\n你发现他的项目文件夹是空的。\n\"先建个文件夹。这是第一步。\"\n\n焦虑 -3（你不孤单）",{L"兄弟一起冲"});
            anxiety-=3; clamp();
        }
        else if(day>=2&&!hasMVP){
            hasMVP=true;
            pushDia(L"👤 老摸 D",L"D 突然正经起来：\n\"兄弟，我研究出一个方法。\n先写一个能跑的类——哪怕只有 name 属性。\n跑通了再加功能。\n别一上来就想写完美的。\"\n\n📜 墙上那张纸也写了：\n\"封装不是为了保护数据，\n 是为了保护你自己——\n 不让人看到你还没写完的屎山。\"\n\n你解锁了：\n💡「先跑通再优化」→ 写代码效率 +3",{L"学到了"});
            addLog(L"💡 解锁 buff: 先跑通再优化"); clamp();
        }
        else {
            pushDia(L"👤 老摸 D",L"D 正在看 B 站教程：\n《30 分钟学会 OOP》\n进度条 0:00 / 30:00\n\n\"我在学习。真的。\"\n\n焦虑 -3",{L"加油吧"});
            anxiety-=3; clamp();
        }
    }

    // ======== 交互：咖啡机 ========
    void doCoffee(){
        if(coffeeCount>=2){
            pushDia(L"☕ 咖啡机",L"你今天已经喝了两杯了。\n咖啡机闪了一下红灯。\n\"够了。\"——它好像在说。\n你手确实有点抖。",{L"好吧"});
            return;
        }
        // 规则0: 空腹喝咖啡
        if(!deskDone&&coffeeCount==0){
            pushDia(L"☕ 规则0 · 空腹喝咖啡",L"你还没写代码就去喝咖啡。\n📜 墙上那纸的第一条：\n\"任何时候都不要空腹喝咖啡。\n 你会胃痛的。\"\n\n你喝了一口——确实。\n\n⚠️ 这个时段被浪费了。",{L"……"});
            coffeeCount++; anxiety+=5; advancePhase(); clamp();
            return;
        }
        coffeeCount++; progress+=2; anxiety+=5;
        addLog(L"☕ 咖啡 +2进度 +5焦虑"); clamp();
        pushDia(L"☕ 咖啡机",L"一杯热咖啡。\n提神了。但你知道这是借来的精力——\n迟早要还的。\n\n📜 规则说：\n\"咖啡不是燃料，是焦虑的催化剂。\"\n\n+2 进度 +5 焦虑",{L"继续肝"});
    }

    // ======== 交互：走廊 ========
    void doDoor(){
        if(day>=4&&deskDone){
            checkEnding();
            if(screen==2) return;
        }
        pushDia(L"🚪 走廊",L"走廊尽头就是提交处。\n\n现在还不到时候。\n你还没写完呢。\n回去。",{L"回去肝"});
    }

    // ======== 交互分发 ========
    void interactObj(MapObj& o){
        o.onInteract(*this); // 多态调度 — 一行替代 7 路 strcmp 分支
    }

    // ======== 选择处理 ========
    void applyChoice(int ci){
        // 深夜重构选项
        if(diaTitle.find(L"深夜写代码")!=std::wstring::npos){
            if(ci==0){ // 写新功能
                progress+=pendingCodeEff; anxiety+=3; addLog(L"💻 深夜写代码 +进度"); clamp();
            } else { // 重构 —— 规则1 惩罚
                progress-=5; anxiety+=10;
                addLog(L"⚠️ 深夜重构! 进度-5 焦虑+10"); clamp();
                pushDia(L"📜 规则1 · 深夜别重构",L"你打开了昨天的代码……\n改了半小时。\n结果——原来能跑的部分也不跑了。\n\n📜 规则第一条：\n\"深夜不要重构代码。\n 你改的每一行都是新的 bug。\"\n\n进度 -5，焦虑 +10",{L"下次不敢了"});
                showDia=false; showChoices=false; advancePhase(); return;
            }
        }
        // B 问你进度（规则5）
        else if(diaTitle.find(L"老卷 B")!=std::wstring::npos
                &&diaText.find(L"吹牛")!=std::wstring::npos){
            if(ci==0){ // 吹牛 → 规则5惩罚
                addLog(L"⚠️ 吹牛被识破 B不理你了"); clamp();
                pushDia(L"📜 规则5 · 别说谎",L"B 看了看你：\n\"快写完了？\n那你屏幕上是空的。\"\n\n他转过头继续写报告。\n不再跟你说话了。\n\n📜 规则第五条：\n\"当室友问你进度时，\n 说谎会遭报应。\"\n\n今天不能再找他了。",{L"……"});
                talkedToB=true; bAskCount++; // 已经是 true，但对话无法再触发
                showDia=false; showChoices=false; advancePhase(); return;
            } else { // 如实回答
                progress+=5; addLog(L"📈 B 指导 +5进度"); clamp();
                pushDia(L"👤 老卷 B",L"B 沉默了一下：\n\"行，至少你诚实。\n我们先看看你的结构……\"\n\n他花了十分钟帮你理清基类派生类。\n\n+5 进度",{L"谢谢"});
            }
        }
        // Day4 床 → 规则4
        else if(diaTitle.find(L"床")!=std::wstring::npos
                &&diaText.find(L"Deadline 前 24 小时")!=std::wstring::npos){
            if(ci==1){ // 睡了
                endingId=3; screen=2; addLog(L"🛏️ 睡过头了…"); clamp();
                showDia=false; showChoices=false; return;
            }
            // 不睡 → 继续
        }

        showDia=false; showChoices=false;
        advancePhase(); clamp(); checkEnding();
    }

    // ======== 每日开场 ========
    void showDayIntro(){
        if(dayIntroShown||day>4) return;
        dayIntroShown=true;
        const wchar_t* titles[]={
            L"📋 Day 1 · 死线预警",L"📋 Day 2 · 开始慌了",L"📋 Day 3 · 绝地求生",L"📋 Day 4 · 最终交稿"};
        const wchar_t* texts[]={
            L"课程群消息：\n「OOP 大作业下周五交。」\n\n你以为是下个月。\n\n你翻身下床——\n老卷在看书（他已经写完了）。\n阿瓜在刷手机。\n老摸在睡觉。\n\n📜 墙上贴着一张泛黄的纸：\n 「Deadline 的时间感知是不准确的。\n  当你以为还有一个月时，\n  实际只剩一周。」\n  ——2019 级 某位学长\n\n你打开了 IDE。\n空白屏幕。闪烁光标。\n4 天。你还有 4 天。",L"昨天你写了声明，今天编译器全报错。\n\n📜 那张纸的第二条：\n 「当你开始怀疑自己有没有上过这门课时——\n  你触发了被动技能：预习。\n  效果：你开始看第一节课的 PPT。\n  副作用：还有 15 个 PPT 没看。」\n\n老卷路过：「你的析构函数呢？」\n你：「什么是析构函数？」\n老卷沉默了。",L"还剩 2 天。\n\n老卷写完了。阿瓜也写完了——他说昨晚写到 3 点。\n老摸也开始了（你发现他偷偷写了个 Student 类）。\n你是宿舍里唯一一个进度还在个位数的人。\n\n📜 那张纸的中间：\n 「每到期末，宿舍出现一种神秘现象——\n  你以为大家都在摸鱼，\n  但其实他们都在你睡觉的时候偷偷卷。」\n\n键盘声像雨点。你没有退路了。",L"交稿日。\n\n📜 最后一条：\n 「交了就有分。\n  不管你的类图多漂亮，\n  不管你的代码能不能跑——\n  交了就有分。」\n\n老卷在改格式。\n阿瓜在吃早饭。\n老摸顶着眼圈——他熬了一夜，写完了。\n\n走廊尽头就是提交处。\n去吧。"};
        pushDia(titles[day-1],texts[day-1],{L"开始"});
    }

    // ======== 渲染：地图 ========
    void drawBar(int x,int y,int w,int h,int val,COLORREF clr){
        setfillcolor(RGB(220,215,205));
        fillrectangle(x,y,x+w,y+h);
        if(val>0){ setfillcolor(clr); fillrectangle(x,y,x+w*val/100,y+h); }
    }
    void drawMap(){
        COLORREF f1=RGB(235,225,210), f2=RGB(225,215,200);
        for(int ty=0;ty<ROWS;ty++) for(int tx=0;tx<COLS;tx++){
            int px=tx*TILE_SZ, py=ty*TILE_SZ, tile=dormMap[ty][tx];
            if(tile==1){ setfillcolor(C_WALL); fillrectangle(px,py,px+TILE_SZ-1,py+TILE_SZ-1); }
            else if(tile==2){ setfillcolor(C_DOOR); fillrectangle(px,py,px+TILE_SZ-1,py+TILE_SZ-1); }
            else { setfillcolor(((tx+ty)%2==0)?f1:f2); fillrectangle(px,py,px+TILE_SZ-1,py+TILE_SZ-1); }
        }
        for(int i=0;i<objCount;i++){
            MapObj* o=objs[i];
            int ox=o->x*TILE_SZ, oy=o->y*TILE_SZ, ow=o->w*TILE_SZ, oh=o->h*TILE_SZ;
            bool isN=nearObj(o,tx,ty);
            if(isN){
                int r=std::min(255,GetRValue(o->clr)+40), g=std::min(255,GetGValue(o->clr)+40), b=std::min(255,GetBValue(o->clr)+40);
                setfillcolor(RGB(r,g,b));
            }else setfillcolor(o->clr);
            fillrectangle(ox,oy,ox+ow-2,oy+oh-2);
            if(isN&&o->solid){ setlinecolor(C_GOLD); rectangle(ox-1,oy-1,ox+ow-1,oy+oh-1); }
            if(o->icon&&wcslen(o->icon)>0){ settextcolor(isN?C_GOLD:C_DIM); settextstyle(isN?26:20,0,L"Microsoft YaHei"); outtextxy(ox+4,oy+2,o->icon); }
            if(o->label&&wcslen(o->label)>0){ settextcolor(isN?C_TEXT:C_DIM); settextstyle(isN?16:14,0,L"Microsoft YaHei"); outtextxy(ox+2,oy+oh+2,o->label); }
        }
        setfillcolor(C_PLAYER); fillcircle(px-1,py-1,7);
        int ex=px, ey=py;
        switch(dir){
            case 0: ex=px;   ey=py-12; break; case 1: ex=px+9; ey=py-9; break;
            case 2: ex=px+12;ey=py;   break; case 3: ex=px+9; ey=py+9; break;
            case 4: ex=px;   ey=py+12;break; case 5: ex=px-9; ey=py+9; break;
            case 6: ex=px-12;ey=py;   break; case 7: ex=px-9; ey=py-9; break;
        }
        setfillcolor(C_PDIR); fillcircle(ex,ey,3);
        if(targetX>=0&&targetY>=0){
            float p=0.6f+0.4f*sinf(tick*0.15f);
            setlinecolor(RGB((int)(255*p),(int)(200*p),50)); circle(targetX,targetY,8); circle(targetX,targetY,4);
        }
        // 底部交互栏
        int barY=MAP_H-55;
        setfillcolor(RGB(252,248,238));
        fillrectangle(0,barY,MAP_W-1,MAP_H-1);
        setlinecolor(C_PANEL_BD); line(0,barY,MAP_W,barY);
        settextstyle(15,0,L"Microsoft YaHei");
        settextcolor(isRunning?C_ORANGE:C_DIM);
        outtextxy(8,barY+6,isRunning?L"🏃 跑步":L"🚶 散步");
        settextcolor(C_DIM); settextstyle(13,0,L"Microsoft YaHei");
        outtextxy(8,barY+28,L"Shift切换"); outtextxy(MAP_W-180,barY+8,L"左键移动 | 点击物体交互"); outtextxy(MAP_W-180,barY+28,L"E键交互 | H标题 | Esc退出");
        // 附近物体交互提示
        MapObj* pObj=findNearObj(tx,ty);
        if(pObj){
            float pulse=0.78f+0.22f*sinf(tick*0.09f);
            settextcolor(RGB((int)(180*pulse),(int)(120*pulse),40)); settextstyle(24,0,L"Microsoft YaHei");
            wchar_t hint[64]; swprintf(hint,64,L"📌 %ls (点击/E)",pObj->label);
            int tw=textwidth(hint); outtextxy((MAP_W-tw)/2,barY+16,hint);
        }
    }

    // ======== 渲染：右侧面板 ========
    void drawPanel(){
        setfillcolor(C_PANEL_BG); fillrectangle(MAP_W,0,W,H);
        setlinecolor(C_PANEL_BD); line(MAP_W,0,MAP_W,MAP_H);
        settextcolor(C_TEXT); settextstyle(26,0,L"Microsoft YaHei"); outtextxy(MAP_W+12,6,timeNames[phase]);
        wchar_t buf[64]; swprintf(buf,64,L"📅 Day %d / 4",day);
        settextstyle(17,0,L"Microsoft YaHei"); settextcolor(C_DIM); outtextxy(MAP_W+15,36,buf);
        // 数值条
        int barY=60;
        settextcolor(C_TEXT); settextstyle(16,0,L"Microsoft YaHei");
        swprintf(buf,64,L"📝 进度 %d/100",progress); outtextxy(MAP_W+15,barY,buf);
        drawBar(MAP_W+15,barY+20,440,6,progress,C_BLUE);
        settextcolor(C_TEXT); settextstyle(16,0,L"Microsoft YaHei");
        swprintf(buf,64,L"😰 焦虑 %d/100",anxiety); outtextxy(MAP_W+15,barY+35,buf);
        drawBar(MAP_W+15,barY+55,440,6,anxiety,C_ORANGE);
        // 分隔 + 室友
        int lineY=barY+75;
        setlinecolor(C_PANEL_BD); line(MAP_W+12,lineY,MAP_W+460,lineY);
        settextcolor(C_TEXT); settextstyle(17,0,L"Microsoft YaHei"); outtextxy(MAP_W+15,lineY+8,L"👥 室友");
        const wchar_t* mNames[]={L"卷王B  老卷",L"瓜王C  阿瓜",L"DDL战士D  老摸"};
        COLORREF mcs[]={RGB(200,140,120),RGB(180,160,140),RGB(140,180,150)};
        for(int i=0;i<3;i++){ settextcolor(mcs[i]); settextstyle(14,0,L"Microsoft YaHei"); outtextxy(MAP_W+15,lineY+32+i*20,mNames[i]); }
        // 分隔 + 日志
        int logY=lineY+32+3*20+10;
        setlinecolor(C_PANEL_BD); line(MAP_W+12,logY,MAP_W+460,logY);
        settextcolor(C_TEXT); settextstyle(17,0,L"Microsoft YaHei"); outtextxy(MAP_W+15,logY+8,L"📜 事件日志");
        for(int i=0;i<(int)eventLog.size();i++){ settextcolor(C_DIM); settextstyle(13,0,L"Microsoft YaHei"); outtextxy(MAP_W+18,logY+30+i*18,eventLog[i].c_str()); }
        // buff 标签
        int tagY=logY+30+6*18+10; if(tagY>MAP_H-100) tagY=MAP_H-100;
        setlinecolor(C_PANEL_BD); line(MAP_W+12,tagY,MAP_W+460,tagY);
        int tc=tagY+8;
        if(hasMVP){ settextcolor(C_GREEN); settextstyle(14,0,L"Microsoft YaHei"); outtextxy(MAP_W+15,tc,L"💡 先跑通再优化 +3效率"); tc+=20; }
        if(progress>=60){ settextcolor(C_GREEN); settextstyle(14,0,L"Microsoft YaHei"); outtextxy(MAP_W+15,tc,L"✅ 进度过半!"); tc+=20; }
        if(anxiety>=80){ settextcolor(C_RED); settextstyle(14,0,L"Microsoft YaHei"); outtextxy(MAP_W+15,tc,L"⚠️ 焦虑过高! 找人聊聊!"); tc+=20; }
        if(codeCount>=2&&!warnedRule2){ settextcolor(C_ORANGE); settextstyle(14,0,L"Microsoft YaHei"); outtextxy(MAP_W+15,tc,L"⚠️ 别再写代码了! 规则2!"); tc+=20; }
    }

    // ======== 渲染：标题画面 ========
    void drawTitle(){
        cleardevice(); setfillcolor(C_BG); fillrectangle(0,0,W,H);
        settextcolor(C_BLUE); settextstyle(52,0,L"Microsoft YaHei");
        int tw=textwidth(L"🎯 Deadline 冲刺器"); outtextxy((W-tw)/2,40,L"🎯 Deadline 冲刺器");
        settextcolor(C_DIM); settextstyle(22,0,L"Microsoft YaHei"); tw=textwidth(L"期末大作业模拟器 · OOP 特别版"); outtextxy((W-tw)/2,105,L"期末大作业模拟器 · OOP 特别版");
        setlinecolor(C_PANEL_BD); int lx=180; line(lx,145,W-lx,145);

        int boxW=420, boxH=360, boxGap=20, boxY=165, totalW=boxW*2+boxGap, leftX=(W-totalW)/2, rightX=leftX+boxW+boxGap;
        setfillcolor(RGB(255,252,243));
        fillrectangle(leftX,boxY,leftX+boxW,boxY+boxH); setlinecolor(C_PANEL_BD); rectangle(leftX,boxY,leftX+boxW,boxY+boxH);
        settextcolor(C_TEXT); settextstyle(19,0,L"Microsoft YaHei"); outtextxy(leftX+24,boxY+14,L"📖 背景");
        setlinecolor(C_PANEL_BD); line(leftX+20,boxY+44,leftX+boxW-20,boxY+44);
        int ly=boxY+60;
        settextcolor(C_DIM); settextstyle(15,0,L"Microsoft YaHei");
        outtextxy(leftX+24,ly,L"大一升大二。期末前最后一周。"); ly+=28;
        outtextxy(leftX+24,ly,L"课程：《面向对象程序设计》"); ly+=26;
        outtextxy(leftX+24,ly,L"大作业：C++ XX管理系统，至少 5 个类。"); ly+=26;
        outtextxy(leftX+24,ly,L"你开学听了前三节，后八节在划水。"); ly+=26;
        outtextxy(leftX+24,ly,L"现在你知道什么是 class——"); ly+=24;
        outtextxy(leftX+24,ly,L"但你不确定自己能写出一个来。"); ly+=30;
        settextcolor(C_ORANGE);
        outtextxy(leftX+24,ly,L"4 天。你还有 4 天。"); ly+=28;
        settextcolor(C_DIM);
        outtextxy(leftX+24,ly,L"同宿舍三个室友也在这条船上——"); ly+=24;
        outtextxy(leftX+24,ly,L"只是每个人的船不一样。"); ly+=28;
        settextcolor(C_GOLD);
        outtextxy(leftX+24,ly,L"📜 墙上贴着一张泛黄的《期末生存法则》…");

        setfillcolor(RGB(255,252,243));
        fillrectangle(rightX,boxY,rightX+boxW,boxY+boxH); setlinecolor(C_GOLD); rectangle(rightX,boxY,rightX+boxW,boxY+boxH);
        settextcolor(C_TEXT); settextstyle(19,0,L"Microsoft YaHei"); outtextxy(rightX+24,boxY+14,L"🎮 操作");
        setlinecolor(C_PANEL_BD); line(rightX+20,boxY+44,rightX+boxW-20,boxY+44);
        int ry=boxY+65;
        settextcolor(C_TEXT); settextstyle(16,0,L"Microsoft YaHei");
        outtextxy(rightX+28,ry,L"🖱️ 左键空地 → 移动"); ry+=30;
        outtextxy(rightX+28,ry,L"🖱️ 左键高亮物体 → 交互"); ry+=30;
        outtextxy(rightX+28,ry,L"⌨️ E键 → 与最近物体交互"); ry+=32;
        outtextxy(rightX+28,ry,L"🏃 Shift → 切换走/跑"); ry+=32;
        outtextxy(rightX+28,ry,L"📋 H键 → 回标题"); ry+=32;
        settextcolor(C_DIM); settextstyle(13,0,L"Microsoft YaHei");
        ry+=15;
        outtextxy(rightX+28,ry,L"💡 跟室友聊天降焦虑"); ry+=22;
        outtextxy(rightX+28,ry,L"💡 先跑通再优化（找老摸D）"); ry+=22;
        outtextxy(rightX+28,ry,L"⚠️ 注意遵守规则——违反有惩罚"); ry+=22;
        outtextxy(rightX+28,ry,L"📜 规则会通过对话体现，不是摆设");

        int btnY=565, btnW=300, btnH=55, btnX=(W-btnW)/2;
        float pulse=0.85f+0.15f*sinf(tick*0.06f);
        setfillcolor(RGB((int)(60*pulse),(int)(120*pulse),(int)(200*pulse)));
        fillrectangle(btnX,btnY,btnX+btnW,btnY+btnH); setlinecolor(RGB(50,100,180)); rectangle(btnX,btnY,btnX+btnW,btnY+btnH);
        settextcolor(RGB(255,255,255)); settextstyle(24,0,L"Microsoft YaHei"); tw=textwidth(L"开 始 冲 刺"); outtextxy((W-tw)/2,btnY+14,L"开 始 冲 刺");
        settextcolor(C_DIM); settextstyle(12,0,L"Microsoft YaHei"); tw=textwidth(L"v1 · C++ OOP | 规则怪谈 | 4天极限冲刺"); outtextxy((W-tw)/2,H-20,L"v1 · C++ OOP | 规则怪谈 | 4天极限冲刺");
    }

    // ======== 渲染：结局画面 ========
    void drawEnding(){
        cleardevice(); setfillcolor(C_BG); fillrectangle(0,0,W,H);
        const wchar_t* titles[]={L"🏆 通关！",L"😅 低空飘过",L"😰 代码屎山",L"🛏️ 摆了"};
        const wchar_t* descs[]={
            L"你的代码跑起来了。\n\n5 个类，有继承，有多态。\n虽然有几个 warning，但它跑起来了。\n\n老师点了点头。\n\n走出教室的时候，老卷在等你。\n「写得不错。」他说。\n\n这是你四天来听过最动听的话。\n\n—— 🏆 真结局：通关 ——",
            L"你交了。\n\n虽然只有 4 个类，虽然继承用反了——\n但是交了。\n\n阿瓜说得对：交了就有分。\n\n老师在群里发了成绩——60 分。\n你看着屏幕，说不清是高兴还是心虚。\n但至少不用补考了。\n\n—— 😅 结局：低空飘过 ——",
            L"你太焦虑了。\n\n手指抖得打不了字。\n最后你交了——\n一份满是注释的代码。\n\n「这里不会」\n「这里也不确定」\n「救命」\n\n老师看着你的代码沉默了。\n「…你至少写了注释。」\n\n—— 😰 结局：代码屎山 ——",
            L"你交了一个几乎空的文件夹。\n\n里面只有一个 Student 类——\n只有 name 属性。\n\n老师在群里 @你：\n「同学，你是不是传错了？」\n\n你没有回复。\n\n阿瓜拍了拍你：\n「没事，下学期还有。」\n老摸：「一起重修。」\n\n—— 🛏️ 结局：摆了 ——"
        };
        COLORREF tc[]={C_GREEN,C_ORANGE,C_RED,RGB(150,150,200)};
        settextcolor(tc[endingId]); settextstyle(46,0,L"Microsoft YaHei"); int tw=textwidth(titles[endingId]); outtextxy((W-tw)/2,30,titles[endingId]);
        settextcolor(C_DIM); settextstyle(16,0,L"Microsoft YaHei"); RECT er={80,100,W-80,440}; DrawTextW(GetImageHDC(),descs[endingId],-1,&er,DT_CENTER|DT_VCENTER|DT_WORDBREAK);
        int btnX=(W-300)/2, btnY=490;
        setfillcolor(RGB(60,120,200)); fillrectangle(btnX,btnY,btnX+300,btnY+55);
        setlinecolor(RGB(50,100,180)); rectangle(btnX,btnY,btnX+300,btnY+55);
        settextcolor(RGB(255,255,255)); settextstyle(22,0,L"Microsoft YaHei"); tw=textwidth(L"重 来 一 次"); outtextxy((W-tw)/2,btnY+14,L"重 来 一 次");
        settextcolor(C_DIM); settextstyle(13,0,L"Microsoft YaHei"); tw=textwidth(L"按R重开 | 按H回标题 | Esc退出"); outtextxy((W-tw)/2,570,L"按R重开 | 按H回标题 | Esc退出");
    }

    // ======== 渲染：对话框 ========
    void drawDialog(){
        int ex=(W-700)/2, ey=40, ew=700, eh=showChoices?540:480;
        setfillcolor(C_DIA_BG); fillrectangle(ex,ey,ex+ew,ey+eh); setlinecolor(C_GOLD); rectangle(ex,ey,ex+ew,ey+eh);
        settextcolor(C_BLUE); settextstyle(23,0,L"Microsoft YaHei"); outtextxy(ex+25,ey+12,diaTitle.c_str());
        setlinecolor(RGB(220,210,195)); line(ex+20,ey+44,ex+ew-20,ey+44);
        settextcolor(C_TEXT); settextstyle(15,0,L"Microsoft YaHei"); RECT tr={ex+30,ey+52,ex+ew-30,ey+300}; DrawTextW(GetImageHDC(),diaText.c_str(),-1,&tr,DT_LEFT|DT_WORDBREAK);
        if(showChoices){ for(int i=0;i<(int)choices.size();i++){ int cy=ey+308+i*44; bool s=(i==hoverSel); setfillcolor(s?C_BTN_H:C_BTN); fillrectangle(ex+35,cy,ex+ew-35,cy+40); setlinecolor(s?C_GOLD:C_PANEL_BD); rectangle(ex+35,cy,ex+ew-35,cy+40); settextcolor(s?C_TEXT:C_DIM); settextstyle(14,0,L"Microsoft YaHei"); outtextxy(ex+50,cy+8,choices[i].c_str()); } }
        int btnX=ex+ew-110, btnY=ey+eh-46, btnW=90, btnH=34;
        setfillcolor(hoverCloseBtn?C_BTN_H:C_BTN); fillrectangle(btnX,btnY,btnX+btnW,btnY+btnH); setlinecolor(hoverCloseBtn?C_GOLD:C_PANEL_BD); rectangle(btnX,btnY,btnX+btnW,btnY+btnH); settextcolor(hoverCloseBtn?C_TEXT:C_DIM); settextstyle(17,0,L"Microsoft YaHei"); int tw=textwidth(L"关闭"); outtextxy(btnX+(btnW-tw)/2,btnY+8,L"关闭");
    }
    // ======== 主循环 ========
    void run(){
        HMODULE u32=GetModuleHandleW(L"user32.dll");
        if(u32){ typedef BOOL(WINAPI*DPIA)(); auto p=(DPIA)GetProcAddress(u32,"SetProcessDPIAware"); if(p)p(); }
        initgraph(W,H);
        HWND hw=FindWindowW(L"EasyXWindow",NULL);
        if(hw) SetWindowTextW(hw,L"Deadline冲刺器");
        BeginBatchDraw(); bool running=true;
        while(running){
            MSG msg;
            while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){
                if(msg.message==WM_QUIT){ running=false; break; }
                if(msg.message==WM_KEYDOWN){
                    int k=(int)msg.wParam;
                    if(k==VK_F11){
                        static bool fs=false; static RECT wr; static LONG ws;
                        HWND hww=FindWindowW(L"EasyXWindow",NULL); if(!hww) continue;
                        if(!fs){ GetWindowRect(hww,&wr); ws=GetWindowLong(hww,GWL_STYLE); SetWindowLong(hww,GWL_STYLE,WS_POPUP|WS_VISIBLE); SetWindowPos(hww,HWND_TOPMOST,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),SWP_FRAMECHANGED|SWP_SHOWWINDOW); fs=true; }
                        else { SetWindowLong(hww,GWL_STYLE,ws); SetWindowPos(hww,HWND_NOTOPMOST,wr.left,wr.top,wr.right-wr.left,wr.bottom-wr.top,SWP_FRAMECHANGED|SWP_SHOWWINDOW); fs=false; }
                    }
                    if(screen==2){ if(k=='R'){ *this=Game(); screen=1; } if(k=='H'){ *this=Game(); screen=0; } if(k==VK_ESCAPE)running=false; }
                    else if(screen==1&&showDia){
                        if(k==VK_ESCAPE){ showDia=false; showChoices=false; hoverSel=-1; }
                        else if(showChoices){
                            if(k==VK_UP){ hoverSel=std::max(0,hoverSel-1); }
                            if(k==VK_DOWN){ hoverSel=std::min((int)choices.size()-1,hoverSel+1); }
                            if(k==VK_RETURN&&hoverSel>=0) applyChoice(hoverSel);
                            if(k>='1'&&k<='9'){ int n=k-'1'; if(n<(int)choices.size()) applyChoice(n); }
                        } else { if(k==VK_RETURN||k=='E'){ showDia=false; showChoices=false; hoverSel=-1; advancePhase(); clamp(); checkEnding(); } }
                    }
                    if(k=='H'&&screen!=0){ *this=Game(); screen=0; }
                    if(k==VK_SHIFT&&screen==1&&!showDia) isRunning=!isRunning;
                }
                if(msg.message==WM_LBUTTONDOWN){
                    int mx=(int)((LOWORD(msg.lParam)-easyx_impl::g_vpX)/easyx_impl::g_scale);
                    int my=(int)((HIWORD(msg.lParam)-easyx_impl::g_vpY)/easyx_impl::g_scale);
                    if(screen==0){ if(inRect(mx,my,(W-300)/2,565,300,55)){ screen=1; showDayIntro(); } }
                    else if(screen==1&&showDia){
                        int ex=(W-700)/2, ey=40, ew=700, eh=showChoices?540:480;
                        int bx=ex+ew-110, by=ey+eh-46, bw=90, bh=34;
                        if(inRect(mx,my,bx,by,bw,bh)){ showDia=false; showChoices=false; hoverSel=-1; }
                        else if(showChoices){ for(int i=0;i<(int)choices.size();i++){ int cy=ey+308+i*44; if(inRect(mx,my,ex+35,cy,ew-70,40)){ applyChoice(i); break; } } }
                    } else if(screen==2){ if(inRect(mx,my,(W-300)/2,490,300,55)){ *this=Game(); screen=1; showDayIntro(); } }
                    else if(screen==1&&!showDia){
                        if(mx>=0&&mx<MAP_W&&my>=0&&my<MAP_H){
                            int cTx=mx/TILE_SZ, cTy=my/TILE_SZ;
                            MapObj* pN=findNearObj(tx,ty);
                            bool interacted=false;
                            if(pN&&cTx>=pN->x&&cTx<pN->x+pN->w&&cTy>=pN->y&&cTy<pN->y+pN->h){ interactObj(*pN); interacted=true; }
                            if(!interacted){
                                int nTx=cTx, nTy=cTy; bool ck=false;
                                for(int i=0;i<objCount;i++){
                                    MapObj* o=objs[i];
                                    if(cTx>=o->x&&cTx<o->x+o->w&&cTy>=o->y&&cTy<o->y+o->h){
                                        int bx=-1,by=-1,bd=9999;
                                        for(int tt=o->y-1;tt<=o->y+o->h;tt++) for(int tx=o->x-1;tx<=o->x+o->w;tx++){
                                            if(canWalk(tx,tt)){ int d=abs(tx-tx)+abs(tt-ty); if(d<bd){bd=d;bx=tx;by=tt;} }
                                        }
                                        if(bx>=0){nTx=bx;nTy=by;ck=true;} break;
                                    }
                                }
                                if(!ck&&!canWalk(nTx,nTy)){
                                    int bx=-1,by=-1,bd=9999;
                                    for(int dy=-1;dy<=1;dy++) for(int dx=-1;dx<=1;dx++){
                                        int ttx=nTx+dx,tty=nTy+dy;
                                        if(canWalk(ttx,tty)){ int d=abs(dx)+abs(dy); if(d<bd){bd=d;bx=ttx;by=tty;} }
                                    }
                                    if(bx>=0){nTx=bx;nTy=by;}
                                }
                                if(tx==nTx&&ty==nTy){ moving=false; moveProgress=0; px=tx*TILE_SZ+TILE_SZ/2; py=ty*TILE_SZ+TILE_SZ/2; targetX=-1; targetY=-1; path.clear(); }
                                else if(targetX>=0&&targetX==nTx*TILE_SZ+TILE_SZ/2&&targetY==nTy*TILE_SZ+TILE_SZ/2){ moving=false; moveProgress=0; px=tx*TILE_SZ+TILE_SZ/2; py=ty*TILE_SZ+TILE_SZ/2; targetX=-1; targetY=-1; path.clear(); }
                                else { if(moving){ moving=false; moveProgress=0; px=tx*TILE_SZ+TILE_SZ/2; py=ty*TILE_SZ+TILE_SZ/2; } path.clear(); targetX=nTx*TILE_SZ+TILE_SZ/2; targetY=nTy*TILE_SZ+TILE_SZ/2; findPath(tx,ty,nTx,nTy,path); }
                            }
                        }
                    }
                }
                if(msg.message==WM_MOUSEMOVE&&showDia){
                    int mx=(int)((LOWORD(msg.lParam)-easyx_impl::g_vpX)/easyx_impl::g_scale);
                    int my=(int)((HIWORD(msg.lParam)-easyx_impl::g_vpY)/easyx_impl::g_scale);
                    int ex=(W-700)/2, ey=40, ew=700, eh=showChoices?540:480;
                    int bx=ex+ew-110, by=ey+eh-46, bw=90, bh=34;
                    hoverCloseBtn=inRect(mx,my,bx,by,bw,bh)?1:0;
                    if(showChoices){ hoverSel=-1; for(int i=0;i<(int)choices.size();i++){ int cy=ey+308+i*44; if(inRect(mx,my,ex+35,cy,ew-70,40)){ hoverSel=i; break; } } }
                }
                TranslateMessage(&msg); DispatchMessageW(&msg);
            }
            if(!running) break; tick++;
            if(screen==1&&!showDia){
                int speed=isRunning?runSpeed:baseSpeed;
                if(moving){
                    moveProgress+=speed;
                    if(moveProgress>=TILE_SZ){
                        tx+=moveDirX; ty+=moveDirY;
                        px=tx*TILE_SZ+TILE_SZ/2; py=ty*TILE_SZ+TILE_SZ/2;
                        moving=false; moveProgress=0;
                        if(day>=4&&isDoorCell(tx,ty)){ checkEnding(); }
                    } else {
                        int sx=tx*TILE_SZ+TILE_SZ/2, sy=ty*TILE_SZ+TILE_SZ/2;
                        px=sx+moveDirX*moveProgress; py=sy+moveDirY*moveProgress;
                    }
                }
                if(!moving){
                    px=tx*TILE_SZ+TILE_SZ/2; py=ty*TILE_SZ+TILE_SZ/2;
                    if(targetX>=0&&!path.empty()){
                        auto& n=path[0];
                        if(tx==n.first&&ty==n.second){ path.erase(path.begin()); if(path.empty()){ targetX=-1;targetY=-1; } }
                        else {
                            int ddx=n.first-tx, ddy=n.second-ty;
                            if(ddx!=0&&ddy!=0){
                                int ttx=tx+(ddx>0?1:-1), tty=ty+(ddy>0?1:-1);
                                if(canWalk(ttx,tty)){
                                    int d=0;
                                    if(ddx>0&&ddy<0)d=1; else if(ddx>0&&ddy==0)d=2; else if(ddx>0&&ddy>0)d=3;
                                    else if(ddx==0&&ddy>0)d=4; else if(ddx<0&&ddy>0)d=5; else if(ddx<0&&ddy==0)d=6; else d=7;
                                    moveDirX=(ddx>0)?1:-1; moveDirY=(ddy>0)?1:-1; dir=d; moving=true; moveProgress=0;
                                }
                            }
                            else if(ddx!=0){ int tt=tx+(ddx>0?1:-1); if(canWalk(tt,ty)){ moveDirX=(ddx>0)?1:-1; moveDirY=0; dir=(ddx>0)?2:6; moving=true; moveProgress=0; } }
                            else if(ddy!=0){ int tt=ty+(ddy>0?1:-1); if(canWalk(tx,tt)){ moveDirX=0; moveDirY=(ddy>0)?1:-1; dir=(ddy>0)?4:0; moving=true; moveProgress=0; } }
                        }
                    }
                }
                bool eD=GetAsyncKeyState('E')&0x8000; static bool eW=false;
                if(eD&&!eW){ eW=true; MapObj*pO=findNearObj(tx,ty); if(pO)interactObj(*pO); }
                if(!eD) eW=false;
                if(!dayIntroShown&&!showDia&&day<=4) showDayIntro();
            }
            cleardevice();
            if(screen==0) drawTitle();
            else if(screen==1){ drawMap(); drawPanel(); }
            else drawEnding();
            if(showDia) drawDialog();
            EndBatchDraw(); Sleep(16);
        }
        closegraph();
    }
};

// ==================== 多态实现 — 7 个派生类的 onInteract ====================
void DeskObj::onInteract(Game& g)   { g.doDesk();   }
void BedObj::onInteract(Game& g)    { g.doBed();    }
void MateBObj::onInteract(Game& g)  { g.talkToB();  }
void MateCObj::onInteract(Game& g)  { g.talkToC();  }
void MateDObj::onInteract(Game& g)  { g.talkToD();  }
void DoorObj::onInteract(Game& g)   { g.doDoor();   }
void CoffeeObj::onInteract(Game& g) { g.doCoffee(); }

int main(){
    Game game;
    game.run();
    return 0;
}
