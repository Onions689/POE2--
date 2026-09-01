#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <windows.h>

using namespace std;

// 控制台颜色函数
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void setRed() { setColor(12); }      // 红色 - 必拿奖励
void setGreen() { setColor(10); }    // 绿色 - 标题
void setYellow() { setColor(14); }   // 黄色 - 提示
void resetColor() { setColor(7); }   // 白色 - 默认

// 攻略条目结构体
struct GuideEntry {
    string mapTraditional;   // 繁体地图名
    string mapSimplified;    // 简体地图名
    string storyReward;      // 剧情内容及奖励
    string seasonReward;     // 赛季玩法奖励
};

// 章节数据结构体
struct ChapterData {
    string title;
    vector<GuideEntry> entries;
};

map<int, ChapterData> chapterMap;

// 判断是否包含必拿标记
bool isMustGet(const string& str) {
    return str.find("【必拿】") != string::npos ||
        str.find("【一定要拿】") != string::npos;
}

// 打印带颜色的文本
void printColoredText(const string& text, int width) {
    string displayText = text;
    if ((int)displayText.length() > width) {
        displayText = displayText.substr(0, width - 3) + "...";
    }

    if (isMustGet(text)) {
        setRed();
        cout << left << setw(width) << displayText;
        resetColor();
    }
    else {
        cout << left << setw(width) << displayText;
    }
}

// 初始化所有攻略数据
void initializeGuides() {
    // ==================== 第一章 ====================
    ChapterData chapter1;
    chapter1.title = "第一章 / Act 1";
    chapter1.entries = {
        {"皆伐", "克里费尔", "击杀BOSS贝拉获得+10%冰冷抗性", "神秘营地开宝箱，1级技能石，蜕变石"},
        {"泥沼陋居", "泥穴", "击杀BOSS回城交任务，1级辅助宝石", "增幅石"},
        {"葛瑞爾林", "韧木森林", "击杀女巫给1级辅助宝石", "无"},
        {"赤谷", "红谷", "找到3个方尖碑并完成，回城提交任务", "2级未切割技能宝石"},
        {"纏縛陰林", "阴森网道", "击杀BOSS获得1级辅助宝石", "3级未切割技能宝石"},
        {"不朽帝國之墓", "永恒园林", "开宝箱获得1个随机戒指", "富豪石"},
        {"配偶的墓室", "伴侣寝宫", "击杀boss配偶获取任务道具", "随机普通品质护身符"},
        {"政務官陵墓", "执政官的地宫", "击杀boss执政官获取任务道具", "随机次级符文"},
        {"獵場", "猎场", "【必拿】击杀BOSS巨鸦，获得2点天赋点", "崇高石"},
        {"弗雷索恩", "茂棘深林", "【必拿】完成3个祭坛，击杀最后BOSS，获得30精魂和一个4级精魂宝石", "1级未切割辅助宝石"},
        {"奧格姆農地", "欧甘农地", "【必拿】找到乌娜的家拿取任务道具，回城提交任务，获得2点天赋", "4级未切割技能宝石"},
        {"奧格姆村", "欧甘村庄", "找到工具回城提交解锁分解台", "工匠石"},
        {"宅第壁壘", "庄园城壁", "解救囚犯获得1级辅助宝石", "5级未切割技能宝石"},
        {"奧格姆宅第", "欧甘庄园", "【必拿】第一层击杀BOSS烛火，获得20最大生命；最后一层击杀伯爵完成第一章", "点金石"}
    };
    chapterMap[1] = chapter1;

    // ==================== 第二章 ====================
    ChapterData chapter2;
    chapter2.title = "第二章 / Act 2";
    chapter2.entries = {
        {"瓦斯提里郊區", "瓦斯提里外沿", "找到BOSS催债魔击杀", "崇高石"},
        {"莫頓挖石場", "莫丹采石场", "这个地图护甲片和磨刀石掉率非常高，找到莫丹矿场进入击杀BOSS解救NPC回城", "5级未切割精魂宝石"},
        {"叛徒之路", "叛徒小径", "【必拿】找到BOSS芭芭拉击杀，解锁升华1；在芭芭拉另外一边找到哈拉尼之门进入", "工匠石"},
        {"哈拉妮關口", "哈拉尼之门", "找到BOSS击杀，艾萨拉如果没跟上可以在记录点重生", "崇高石"},
        {"乳齒象惡地", "长毛象荒原", "找到暗光走廊解锁深渊工艺（可后续再来），找到遗迹深坑进入", "富豪石"},
        {"骨坑", "遗迹深坑", "打怪掉落任务道具，找到BOSS击杀获取任务道具回城", "崇高石"},
        {"凱斯城", "克斯", "【必拿】击杀BOSS蛇女，获得2点天赋；杀怪掉落另外一个任务道具；地图边缘小房间开宝箱给项链；找到失落之城进入", "宝石匠棱镜"},
        {"失落之城", "无", "找到掩埋的殿堂进入", "点金石"},
        {"掩埋神殿", "掩埋的殿堂", "元素神殿给对应抗性的戒指，找到克斯之心进入，击杀BOSS对话水之女神拿道具回城", "低级工匠石【一定要拿】"},
        {"泰坦之谷", "巨人之谷", "找到3个封印激活解锁中间的巨人石窟，找到传送阵激活并提交任务道具获取护符增益，进入巨人石窟", "随机暗金装备"},
        {"泰坦石窟", "巨人石窟", "击杀BOSS神威巨像获得任务道具", "机会石碎片"},
        {"戴斯哈", "德莎尔", "找到遗体互动获得遗书，回城提交任务获得2点天赋；找到怀念之阶进入，再找德莎尔高塔进入", "随机符文"},
        {"戴斯哈尖塔", "德莎尔的高塔", "找到格鲁坎的姐妹先不互动，找到BOSS激活传送点再传回去互动，【必拿】获得+10闪电抗性；击杀BOSS污染者；最后前往无畏队击杀关底BOSS结束第二章", "宝石匠棱镜"}
    };
    chapterMap[2] = chapter2;

    // ==================== 第三章 ====================
    ChapterData chapter3;
    chapter3.title = "第三章 / Act 3";
    chapter3.entries = {
        {"風沙沼澤", "飞沙沼泽", "找到篝火营地开箱子获得低级工匠石，找到金字塔营地进入", "3级未切割辅助宝石"},
        {"叢林遺跡", "丛林废墟", "【必拿】击杀BOSS神威银拳，获得2点天赋；找到传送点激活，进入附近的蛇毒地窟；找到滋孽荒原进入", "点金石"},
        {"劇毒墓穴", "蛇毒地窟", "找到骸蛇毒液回城对话获得奖励（建议拿眩晕门槛）", "随机戒指"},
        {"感染荒地", "滋孽荒原", "第三章地图岔路很多，先激活传送点一个一个完成；找到感染荒地传送点激活；找到阿扎卡泥沼进入；找到幻缈湿地进入", "崇高石"},
        {"阿札克泥沼", "无", "【必拿】击杀BOSS获得30精魂和精魂技能宝石", "随机符文"},
        {"龍蜥濕地", "幻缈湿地", "找到混沌神殿解锁混沌试炼（升华2）；找到BOSS奇美拉击杀，进入BOSS后面的机械迷城", "9级未切割技能宝石"},
        {"吉卡尼的機械迷城", "佳华尼的机械迷城", "收集任务道具开门，找到BOSS黑鳄的房间击杀，【必拿】获得+10%火焰抗性；找到佳华尼的密殿进入", "工匠石"},
        {"吉卡尼的聖域", "佳华尼的密殿", "找到2个灵核激活发电机，回到进门的地方击杀BOSS，获得大灵核传送到滋孽荒原插入阵法，进入马特兰水道", "崇高石"},
        {"瑪特蘭水道", "马特兰水道", "拉杆走到最后回城走到地图右下方进入淹没之城", "10级未切割精魂宝石"},
        {"淹沒之城", "淹没之城", "沿着马路一路走到污秽巅峰，中途找到熔火宝库激活传送点", "3级未切割辅助宝石"},
        {"熔岩寶庫", "熔火宝库", "击杀BOSS完成任务获得重铸台（战斗难度高建议之后做）", "随机暗金装备"},
        {"污垢頂峰", "污秽巅峰", "击杀BOSS污秽女王获得任务道具，插进阿尔瓦后面的大门进入科佩克之殿", "瓦尔宝珠"},
        {"科佩克神殿", "科佩克之殿", "找到BOSS击杀进入崎点回到过去", "11级未切割精魂宝石"},
        {"奧札爾", "乌扎尔", "沿着大马路走击杀BOSS毒蛇女王，找阿戈拉特进入（这两张图杀小怪爆奉献之核）；在阿戈拉特找到祭坛完成献祭【必拿】获得2点天赋", "随机珠宝/时跌珠宝"},
        {"漆黑密室", "暗胧殿堂", "击杀关底BOSS结束第三章", "瓦尔宝珠"}
    };
    chapterMap[3] = chapter3;

    // ==================== 第四章 ====================
    ChapterData chapter4;
    chapter4.title = "第四章 / Act 4";
    chapter4.entries = {
        {"凱吉灣", "落锚湾", "第四章任务非线性，找到旅途终点进入", "崇高石"},
        {"旅程之末", "旅途终点", "击杀BOSS获得维金，回城对话获取维金长钉；前往落锚湾完成任务，【必拿】获得2点天赋", "点金石"},
        {"伯勞鳥之島", "百舌岛", "找到BOSS击杀，对话NPC后回城，解锁辛格拉之眼", "4级未切割辅助宝石"},
        {"悉妮蔻拉之眼", "辛格拉之眼", "完成3个试炼，找到沉默之井互动获得5%最大魔力；找到亡者之境进入", "混沌石"},
        {"亡者之殿", "亡者之境", "完成3个试炼获得空白文身（可自选5%抗性或5属性）；击杀最后BOSS获得任务道具往后面进入对话，【必拿】获得2点天赋", "随机符文"},
        {"金氏島", "亲眷岛", "找到熔火院落进入", "宝石匠棱镜"},
        {"火山迷窟", "熔火院落", "找到BOSS击杀", "4级未切割辅助宝石"},
        {"瓦卡帕努島", "瓦卡帕努岛", "支线都可不做，找到冥曲洞窟进入", "工匠石"},
        {"吟謠洞窟", "冥曲洞窟", "找到蚌壳获得珍珠，回城换全抗项链；找到BOSS击杀对话后回城", "随机护符"},
        {"廢棄監獄", "废弃监牢", "杀怪掉落教堂钥匙，激活女神选择药水buff；找到遗世监禁进入", "崇高石"},
        {"單獨禁閉室", "遗世监禁", "找到BOSS击杀（需要用周围机关才能干掉他）", "随机符文"},
        {"阿拉塔斯", "阿拉斯塔", "击杀BOSS进入挖掘场", "12级未切割技能宝石"},
        {"挖掘", "挖掘场", "找到并击杀BOSS", "随机稀有护身符"},
        {"尼加卡努", "纳卡努", "赛季玩法奖励一定要拿，找到并进入部族之心", "高阶工匠石"},
        {"部族之心", "部族之心", "击杀BOSS对话回城第四章结束", "12级未切割精魂宝石"}
    };
    chapterMap[4] = chapter4;

    // ==================== 间奏一章 ====================
    ChapterData interlude1;
    interlude1.title = "间奏一章 / Interlude A1";
    interlude1.entries = {
        {"火噬農地", "焦土农田", "间章非线性，如果怕打不过王子可先打第二章间章；找到BOSS击杀，找到黑暗入口激活记录点，找到并进入瑟尔之石", "4级未切割辅助宝石"},
        {"瑟雷之石", "瑟尔之石", "找到6个石柱激活，击杀中间BOSS，和乌娜对话；回到焦土农田进入黑暗入口-黑木林", "崇高石"},
        {"黑木林", "黑木林", "找到霍尔顿进入", "高阶蜕变石"},
        {"霍爾登", "霍尔顿", "找到狼堡进入，找到霍尔顿庄园进入", "随机高阶符文"},
        {"狼之要塞", "狼堡", "【必拿】击杀BOSS获得2点天赋", "高阶增幅石"},
        {"霍爾登宅第", "霍尔顿庄园", "击杀BOSS回城前往下一章", "工匠石"}
    };
    chapterMap[5] = interlude1;

    // ==================== 间奏二章 ====================
    ChapterData interlude2;
    interlude2.title = "间奏二章 / Interlude A2";
    interlude2.entries = {
        {"卡里交匯道", "卡莉渡口", "这张地图是交通枢纽，需完成4个事情：加莱之门激活传送点；卡哈塔之池激活传送点；骨鳄阶梯获得5%最大生命；找到吞噬兽和蝎子击杀，回城提交任务【必拿】获得2点天赋", "宝石匠棱镜"},
        {"卡塔爾之塘", "卡哈塔之池", "进入塞卡莉圣所", "点金石"},
        {"塞爾卡里庇護所", "塞卡莉圣所", "支线可获得珠宝、戒指、项链（有需要可拿），找到BOSS击杀", "机会石"},
        {"賈萊關口", "加莱之门", "找到BOSS击杀进入奇玛尔", "高阶增幅石"},
        {"奇瑪", "奇玛尔", "地图边缘小房间有7个柱子任选一个属性激活（可随时更换），找到齐玛尔水库进入", "崇高石"},
        {"奇瑪水源地", "奇玛尔水库", "击杀BOSS王子回城对话进入最后一章", "高阶蜕变石"}
    };
    chapterMap[6] = interlude2;

    // ==================== 间奏三章 ====================
    ChapterData interlude3;
    interlude3.title = "间奏三章 / Interlude A3";
    interlude3.entries = {
        {"灰燼森林", "灰烬森林", "找到克里亚村进入", "随机稀有腰带"},
        {"庫萊亞村", "克里亚村", "【必拿】击杀BOSS毒蛇女王二世获得40精魂，进入冰川池塘", "随机高阶符文"},
        {"冰川湖泊", "冰川池塘", "击杀BOSS进入克里亚峰，找到嚎叫洞穴", "高阶增幅石"},
        {"狂嗥洞穴", "嚎叫洞穴", "【必拿】击杀BOSS获得2点天赋", "混沌石"},
        {"庫萊亞山巔", "克里亚峰", "找到长老领取暗金装备一件（猎首腰带来了），找到并进入刻蚀峡谷", "高阶蜕变石"},
        {"蝕刻溪谷", "刻蚀峡谷", "击杀BOSS进入库阿奇克宝库", "崇高石"},
        {"庫阿西克寶庫", "库阿奇克宝库", "击杀BOSS，所有剧情结束进入异界，黑袍给你【必拿】最后2点天赋", "瓦尔宝珠"}
    };
    chapterMap[7] = interlude3;
}

// 显示菜单
void showMenu() {
    setGreen();
    cout << "\n========================================" << endl;
    cout << "      流放之路 剧情攻略查询系统" << endl;
    cout << "========================================" << endl;
    resetColor();
    cout << "请输入对应的数字代码查询攻略：" << endl;
    cout << "  1 - 第一章" << endl;
    cout << "  2 - 第二章" << endl;
    cout << "  3 - 第三章" << endl;
    cout << "  4 - 第四章" << endl;
    cout << "  5 - 间奏一章" << endl;
    cout << "  6 - 间奏二章" << endl;
    cout << "  7 - 间奏三章" << endl;
    setYellow();
    cout << "  0 - 退出程序" << endl;
    resetColor();
    cout << "========================================" << endl;
    cout << "请选择: ";
}

// 打印章节攻略
void printChapter(int chapterId) {
    auto it = chapterMap.find(chapterId);
    if (it == chapterMap.end()) {
        cout << "无效的章节编号！" << endl;
        return;
    }

    const ChapterData& chapter = it->second;

    setGreen();
    cout << "\n========== " << chapter.title << " 攻略 ==========" << endl;
    resetColor();

    // 打印表头
    cout << left
        << setw(6) << "序号"
        << setw(18) << "地图名(繁)"
        << setw(18) << "地图名(简)"
        << setw(52) << "剧情内容及奖励"
        << setw(32) << "赛季玩法奖励"
        << endl;
    cout << string(126, '-') << endl;

    int index = 1;
    for (const auto& entry : chapter.entries) {
        // 序号
        cout << setw(6) << index++;

        // 地图名(繁)
        string trad = entry.mapTraditional;
        if (trad.length() > 18) trad = trad.substr(0, 15) + "...";
        cout << setw(18) << trad;

        // 地图名(简)
        string simp = entry.mapSimplified.empty() ? "无" : entry.mapSimplified;
        if (simp.length() > 18) simp = simp.substr(0, 15) + "...";
        cout << setw(18) << simp;

        // 剧情奖励（自动标红）
        string story = entry.storyReward;
        if (story.length() > 50) story = story.substr(0, 47) + "...";
        if (isMustGet(entry.storyReward)) {
            setRed();
            cout << setw(52) << story;
            resetColor();
        }
        else {
            cout << setw(52) << story;
        }

        // 赛季奖励（自动标红）
        string season = entry.seasonReward.empty() ? "无" : entry.seasonReward;
        if (season.length() > 30) season = season.substr(0, 27) + "...";
        if (isMustGet(entry.seasonReward)) {
            setRed();
            cout << setw(32) << season;
            resetColor();
        }
        else {
            cout << setw(32) << season;
        }

        cout << endl;
    }

    setYellow();
    cout << "========================================" << endl;
    cout << "提示：红色文字为【必拿】或【一定要拿】的奖励，请务必完成！" << endl;
    resetColor();
}

// 主函数
int main() {
    initializeGuides();

    int choice;
    while (true) {
        showMenu();
        cin >> choice;

        if (choice == 0) {
            setYellow();
            cout << "\n感谢使用，再见！" << endl;
            resetColor();
            break;
        }
        else if (choice >= 1 && choice <= 7) {
            printChapter(choice);
        }
        else {
            cout << "输入无效，请输入1-7之间的数字，或输入0退出。" << endl;
        }

        cout << "\n按回车键继续...";
        cin.ignore();
        cin.get();
    }

    return 0;
}