#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <iomanip>
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>

using namespace std;

// ==================== 辅助转换函数 ====================
static string WStringToUtf8(const wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 0) return "";
    string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

static string WStringToGbk(const wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 0) return "";
    string result(len - 1, '\0');
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

static wstring Utf8ToWString(const string& utf8Str) {
    if (utf8Str.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (len <= 0) return L"";
    wchar_t* wstr = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wstr, len);
    wstring result(wstr);
    delete[] wstr;
    return result;
}

// ==================== 全局变量 ====================
HWND g_overlayHwnd = NULL;
bool g_overlayMode = false;
wstring g_currentMap = L"等待游戏启动...";
wstring g_currentGuide = L"暂无攻略\n进入游戏后自动显示";
HANDLE g_monitorThread = NULL;
wstring g_gameLogPath = L"";

int g_windowWidth = 420;
int g_windowHeight = 220;
int g_windowX = -1;
int g_windowY = -1;

struct GuideEntry {
    wstring mapName;
    wstring storyReward;
    wstring seasonReward;
};
struct ChapterData {
    wstring title;
    vector<GuideEntry> entries;
};
map<int, ChapterData> chapterMap;
map<wstring, wstring> guideMap;

static bool fileExists(const wstring& path) {
    DWORD attr = GetFileAttributesW(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}
// 日志路径配置文件
static const string LOG_PATH_CONFIG = "poe2_log_path.txt";



// ==================== 保存/读取日志路径配置 ====================
static void SaveLogPath(const wstring& path) {
    ofstream config(LOG_PATH_CONFIG);
    if (config.is_open()) {
        string utf8Path = WStringToUtf8(path);
        config << utf8Path;
        config.close();
    }
}

static wstring LoadLogPath() {
    ifstream config(LOG_PATH_CONFIG);
    if (config.is_open()) {
        string utf8Path;
        getline(config, utf8Path);
        config.close();
        if (!utf8Path.empty()) {
            wstring path = Utf8ToWString(utf8Path);
            if (fileExists(path)) {
                return path;
            }
        }
    }
    return L"";
}

// ==================== 保存/读取窗口配置 ====================
static void SaveWindowConfig() {
    ofstream config("poe2_overlay_config.txt");
    if (config.is_open()) {
        config << g_windowWidth << endl;
        config << g_windowHeight << endl;
        config << g_windowX << endl;
        config << g_windowY << endl;
        config.close();
    }
}

static void LoadWindowConfig() {
    ifstream config("poe2_overlay_config.txt");
    if (config.is_open()) {
        config >> g_windowWidth;
        config >> g_windowHeight;
        config >> g_windowX;
        config >> g_windowY;
        config.close();
        if (g_windowWidth < 350) g_windowWidth = 350;
        if (g_windowWidth > 800) g_windowWidth = 800;
        if (g_windowHeight < 180) g_windowHeight = 180;
        if (g_windowHeight > 600) g_windowHeight = 600;
    }
}

static void OpenUrl(const wstring& url) {
    ShellExecuteW(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

static wstring ToSimplified(const wstring& trad) {
    static map<wstring, wstring> tradToSimp = {
        {L"河岸",L"河岸"},{L"獵場",L"猎场"},{L"纏縛陰林",L"缠缚阴林"},{L"奧格姆農地",L"奥格姆农地"},
        {L"奧格姆村",L"奥格姆村"},{L"奧格姆宅第",L"奥格姆宅第"},{L"宅第壁壘",L"宅第壁垒"},
        {L"不朽帝國之墓",L"不朽帝国之墓"},{L"政務官陵墓",L"政务官陵墓"},{L"配偶的墓室",L"配偶的墓室"},
        {L"弗雷索恩",L"弗雷索恩"},{L"葛瑞爾林",L"葛瑞尔林"},{L"泥沼陋居",L"泥沼陋居"},
        {L"皆伐",L"皆伐"},{L"赤谷",L"赤谷"},{L"瓦斯提里郊區",L"瓦斯提里郊区"},
        {L"阿杜拉車隊",L"阿杜拉的商队"},{L"莫頓挖石場",L"莫顿挖石场"},{L"莫頓礦坑",L"莫顿矿坑"},
        {L"叛徒之路",L"叛徒之路"},{L"哈拉妮關口",L"哈拉妮关口"},{L"乳齒象惡地",L"乳齿象恶地"},
        {L"骨坑",L"骨坑"},{L"凱斯城",L"凯斯城"},{L"失落之城",L"失落之城"},
        {L"掩埋神殿",L"掩埋神殿"},{L"泰坦之谷",L"泰坦之谷"},{L"泰坦石窟",L"泰坦石窟"},
        {L"戴斯哈",L"戴斯哈"},{L"戴斯哈尖塔",L"戴斯哈尖塔"},{L"悼念之路",L"悼念之路"},
        {L"無畏隊",L"无畏队"},{L"風沙沼澤",L"风沙沼泽"},{L"高地神塔營地",L"高地神塔营地"},
        {L"叢林遺跡",L"丛林遗迹"},{L"劇毒墓穴",L"剧毒墓穴"},{L"感染荒地",L"感染荒地"},
        {L"阿札克泥沼",L"阿札克泥沼"},{L"龍蜥濕地",L"龙蜥湿地"},
        {L"吉卡尼的機械迷城",L"吉卡尼的机械迷城"},{L"吉卡尼的聖域",L"吉卡尼的圣域"},
        {L"瑪特蘭水道",L"玛特兰水道"},{L"淹沒之城",L"淹没之城"},{L"熔岩寶庫",L"熔岩宝库"},
        {L"污垢頂峰",L"污垢顶峰"},{L"科佩克神殿",L"科佩克神殿"},{L"奧札爾",L"奥札尔"},
        {L"暗霧殿堂",L"暗雾殿堂"},{L"金司馬區",L"金司马区"},{L"凱吉灣",L"凯吉湾"},
        {L"旅程之末",L"旅程之末"},{L"伯勞鳥之島",L"伯劳鸟之岛"},{L"悉妮蔻拉之眼",L"悉妮蔻拉之眼"},
        {L"亡者之殿",L"亡者之殿"},{L"祖靈的試煉",L"祖灵的试炼"},{L"金氏島",L"金氏岛"},
        {L"火山迷窟",L"火山迷窟"},{L"瓦卡帕努島",L"瓦卡帕努岛"},{L"吟謠洞窟",L"吟谣洞窟"},
        {L"掠奪者之角",L"掠夺者之角"},{L"廢棄監獄",L"废弃监牢"},{L"單獨禁閉室",L"单独禁闭室"},
        {L"阿拉塔斯",L"阿拉塔斯"},{L"挖掘",L"挖掘"},{L"尼加卡努",L"尼加卡努"},
        {L"部族之心",L"部族之心"},{L"庇護所",L"庇护所"},{L"火噬農地",L"火噬农地"},
        {L"瑟雷之石",L"瑟雷之石"},{L"黑木林",L"黑木林"},{L"霍爾登",L"霍尔登"},
        {L"狼之要塞",L"狼之要塞"},{L"霍爾登宅第",L"霍尔登宅第"},{L"卡里市集",L"卡里集市"},
        {L"卡里交匯道",L"卡里交汇道"},{L"卡塔爾之塘",L"卡塔尔之塘"},
        {L"塞爾卡里庇護所",L"塞尔卡里庇护所"},{L"賈萊關口",L"贾莱关口"},{L"奇瑪",L"奇玛"},
        {L"奇瑪水源地",L"奇玛水源地"},{L"林間空地",L"林间空地"},{L"灰燼森林",L"灰烬森林"},
        {L"庫萊亞村",L"库莱亚村"},{L"冰川湖泊",L"冰川湖泊"},{L"狂嗥洞穴",L"狂嗥洞穴"},
        {L"庫萊亞山巔",L"库莱亚山巅"},{L"蝕刻溪谷",L"蚀刻溪谷"},{L"庫阿西克寶庫",L"库阿西克宝库"}
    };
    auto it = tradToSimp.find(trad);
    if (it != tradToSimp.end()) return it->second;
    return trad;
}

static wstring ToTraditional(const wstring& simp) {
    static map<wstring, wstring> simpToTrad = {
        {L"猎场",L"獵場"},{L"缠缚阴林",L"纏縛陰林"},{L"奥格姆农地",L"奧格姆農地"},
        {L"奥格姆村",L"奧格姆村"},{L"奥格姆宅第",L"奧格姆宅第"},{L"宅第壁垒",L"宅第壁壘"},
        {L"不朽帝国之墓",L"不朽帝國之墓"},{L"政务官陵墓",L"政務官陵墓"},{L"配偶的墓室",L"配偶的墓室"},
        {L"弗雷索恩",L"弗雷索恩"},{L"葛瑞爾林",L"葛瑞爾林"},{L"泥沼陋居",L"泥沼陋居"},
        {L"皆伐",L"皆伐"},{L"赤谷",L"赤谷"}
    };
    auto it = simpToTrad.find(simp);
    if (it != simpToTrad.end()) return it->second;
    return simp;
}

static void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
static void setRed() { setColor(12); }
static void setGreen() { setColor(10); }
static void setYellow() { setColor(14); }
static void resetColor() { setColor(7); }



// ==================== 通过进程自动获取国际服日志路径 ====================
static wstring getLogPathFromProcess() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return L"";
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return L"";
    }
    wstring result = L"";
    wchar_t exePathW[MAX_PATH];
    do {
        wstring processName = pe32.szExeFile;
        for (auto& c : processName) c = towlower(c);
        if (processName == L"pathofexile.exe" || processName == L"pathofexile_x64.exe") {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                DWORD size = MAX_PATH;
                if (QueryFullProcessImageNameW(hProcess, 0, exePathW, &size)) {
                    wstring fullPath(exePathW);
                    if (fullPath.find(L"WeGameApps") == wstring::npos &&
                        fullPath.find(L"rail_apps") == wstring::npos &&
                        fullPath.find(L"流放之路") == wstring::npos) {
                        size_t lastSlash = fullPath.find_last_of(L"\\");
                        if (lastSlash != wstring::npos) {
                            wstring gameDir = fullPath.substr(0, lastSlash);
                            wstring logPath = gameDir + L"\\logs\\LatestClient.txt";
                            DWORD attr = GetFileAttributesW(logPath.c_str());
                            if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                                result = logPath;
                                CloseHandle(hProcess);
                                break;
                            }
                            logPath = gameDir + L"\\logs\\Client.txt";
                            attr = GetFileAttributesW(logPath.c_str());
                            if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                                result = logPath;
                                CloseHandle(hProcess);
                                break;
                            }
                        }
                    }
                }
                CloseHandle(hProcess);
            }
        }
    } while (Process32NextW(hSnapshot, &pe32));
    CloseHandle(hSnapshot);
    return result;
}

// ==================== 获取日志路径（自动检测 > 配置文件 > 手动输入）====================
static wstring getLogPath() {
    // 1. 先自动检测游戏进程（最准确）
    wstring logPath = getLogPathFromProcess();
    if (!logPath.empty()) {
        setGreen();
        cout << "[自动] 检测到国际服日志路径: " << WStringToGbk(logPath) << endl;
        resetColor();
        SaveLogPath(logPath);
        return logPath;
    }

    // 2. 自动检测失败，尝试读取配置文件中的路径
    wstring savedPath = LoadLogPath();
    if (!savedPath.empty() && fileExists(savedPath)) {
        setGreen();
        cout << "[自动] 使用上次保存的日志路径: " << WStringToGbk(savedPath) << endl;
        resetColor();
        return savedPath;
    }

    // 3. 配置文件也无效，询问用户是否手动输入
    setYellow();
    cout << "\n[提示] 自动检测失败，且无有效的保存路径。" << endl;
    cout << "是否手动输入日志路径？(Y/N): ";
    resetColor();

    string choice;
    getline(cin, choice);
    for (auto& c : choice) c = tolower(c);

    if (choice == "y" || choice == "yes") {
        // 手动输入循环
        while (true) {
            setYellow();
            cout << "\n请手动输入日志文件的完整路径：" << endl;
            cout << "示例：D:\\Game\\POE 2\\logs\\LatestClient.txt" << endl;
            cout << "（输入 0 返回主菜单）" << endl;
            cout << "请输入路径: ";
            resetColor();

            string input;
            getline(cin, input);

            if (input == "0") {
                return L"";
            }

            // 去除首尾空格和引号
            while (!input.empty() && (input.back() == ' ' || input.back() == '\"')) input.pop_back();
            while (!input.empty() && input.front() == '\"') input.erase(0, 1);

            if (input.empty()) {
                setRed();
                cout << "[错误] 路径不能为空，请重新输入。" << endl;
                resetColor();
                continue;
            }

            wstring wpath = Utf8ToWString(input);
            if (fileExists(wpath)) {
                setGreen();
                cout << "[OK] 路径有效，已保存。" << endl;
                resetColor();
                SaveLogPath(wpath);
                return wpath;
            }
            else {
                setRed();
                cout << "[错误] 文件不存在，请检查路径是否正确。" << endl;
                resetColor();
            }
        }
    }
    else {
        // 用户选择 N，输出错误提示并返回主菜单
        setRed();
        cout << "\n[错误] 未找到国际服游戏进程或日志文件！" << endl;
        cout << "请确保国际服游戏正在运行，或下次运行时选择手动输入。" << endl;
        resetColor();
        return L"";
    }
}

// ==================== 初始化攻略数据 ====================
static void initializeChapters() {
    // 第一章
    ChapterData ch1;
    ch1.title = L"第一章 / Act 1";
    ch1.entries = {
        {L"河岸", L"击杀BOSS浮肿米勒，点天赋，前往皆伐营地", L"无"},
        {L"皆伐", L"贴墙跑找神秘营地拿技能石；击杀BOSS贝拉+10%冰抗；跑图去葛瑞尔林", L"无"},
        {L"泥沼陋居", L"击杀BOSS吞噬者，拿宝石，回城交任务", L"无"},
        {L"葛瑞爾林", L"开人村下传送点；去缠缚阴林开传送点后返回；跑图去赤谷", L"符文树（最终解锁入口）"},
        {L"赤谷", L"开3个方尖碑，击杀BOSS锈污之王，回城交任务拿符文钉子", L"无"},
        {L"纏縛陰林", L"击杀BOSS腐烂的德鲁伊，拿1级辅助宝石；跑图去不朽帝国之墓", L"【必拿】符文点2"},
        {L"不朽帝國之墓", L"贴边开图找技能点戒指；找配偶和执政官入口", L"无"},
        {L"配偶的墓室", L"击杀BOSS阿席妮雅，获取任务道具钥匙", L"无"},
        {L"政務官陵墓", L"击杀BOSS德雷文永恒执政官，获取任务道具", L"无"},
        {L"獵場", L"【必拿】击杀BOSS悬铃巨鸦，获得2点天赋点", L"【必拿】符文点（狂野符文石）"},
        {L"弗雷索恩", L"【必拿】击杀BOSS迷雾之王，获得30精魂", L"无"},
        {L"奧格姆農地", L"【必拿】找到乌娜的小屋，获得2点天赋点；跑图去奥格姆村", L"【必拿】符文点1"},
        {L"奧格姆村", L"找铁匠工具解锁分解台；击杀BOSS刽子手；跑图去宅第壁垒", L"无"},
        {L"宅第壁壘", L"解救囚犯拿1级辅助宝石；跑图去奥格姆宅第", L"无"},
        {L"奧格姆宅第", L"【必拿】第一层击杀BOSS烛火+20最大生命；第三层击杀伯爵通关", L"无"}
    };
    chapterMap[1] = ch1;

    // 第二章
    ChapterData ch2;
    ch2.title = L"第二章 / Act 2";
    ch2.entries = {
        {L"瓦斯提里郊區", L"击杀BOSS摧寨魔；回城找扎卡对话上车", L"无"},
        {L"阿杜拉車隊", L"点击桌子去哈拉妮关口；找所萨拉对话后回城；点击桌子去采石场", L"无"},
        {L"莫頓挖石場", L"跑图去莫顿矿坑击杀BOSS；与NPC对话后回城；点击桌子去叛徒之路", L"无"},
        {L"莫頓礦坑", L"击杀BOSS恐惧工程师鲁贾", L"无"},
        {L"叛徒之路", L"【必拿】击杀BOSS叛徒芭芭拉，解锁升华试炼", L"无"},
        {L"哈拉妮關口", L"击杀BOSS崛起之王贾曼拉；找记录点回城；点击桌子去凯斯城", L"无"},
        {L"乳齒象惡地", L"跑去骨坑；路过开启灵魂之井", L"无"},
        {L"骨坑", L"击杀BOSS远古之蹄埃克巴勃；回城点击桌子去泰坦之谷", L"【必拿】符文点1（左上方）"},
        {L"凱斯城", L"【必拿】击杀BOSS蟒蛇女王卡巴拉，获得2点天赋；跑图去失落之城", L"【必拿】符文点2（传送点往下）"},
        {L"失落之城", L"跑图去掩埋神殿", L"无"},
        {L"掩埋神殿", L"击杀BOSS遗忘之子阿萨里恩；回城点击桌子去荒原", L"无"},
        {L"泰坦之谷", L"找到远古誓言圣物镶嵌台（护符栏+1）；开3个封印进石窟", L"无"},
        {L"泰坦石窟", L"击杀BOSS巨像札尔玛拉斯；回城点击桌子去哈拉妮关口", L"第二章符文解锁副本"},
        {L"戴斯哈", L"【必拿】找到遗书，回城交任务获得2点天赋；跑图去悼念之路", L"【必拿】符文点3（进入宝库）"},
        {L"悼念之路", L"跑图去戴斯哈尖塔", L"无"},
        {L"戴斯哈尖塔", L"【必拿】找到卡洛翰的姐妹+10%闪电抗性；击杀BOSS", L"无"},
        {L"無畏隊", L"击杀BOSS憎恶者贾曼拉；回城找老黑对话结束第二章", L"无"}
    };
    chapterMap[2] = ch2;

    // 第三章
    ChapterData ch3;
    ch3.title = L"第三章 / Act 3";
    ch3.entries = {
        {L"風沙沼澤", L"找到击杀BOSS根淤；拿低阶工匠石；跑图去高地神塔营地", L"无"},
        {L"高地神塔營地", L"对话接任务后去丛林遗迹", L"无"},
        {L"叢林遺跡", L"【必拿】击杀BOSS神威银拳，获得2点天赋；找到剧毒墓穴入口点亮记录点；跑图去荒地", L"无"},
        {L"劇毒墓穴", L"找到奥洛克毒素；拿剧毒饮剂（三选一）", L"无"},
        {L"感染荒地", L"点亮水道旁传送点；跑图去龙蜥湿地", L"符文点3：进入神秘避难所"},
        {L"龍蜥濕地", L"找到混沌神殿开传送点；击杀BOSS龙蜥钦路锡安；开迷城传送点", L"无"},
        {L"吉卡尼的機械迷城", L"找尸体得血液回城领药；找灵魂核心开任务；击杀BOSS遗世黑颚+10%火抗", L"【必拿】符文点2"},
        {L"吉卡尼的聖域", L"找2个中型灵核开电机发电；传送门口击杀BOSS核心守卫", L"无"},
        {L"瑪特蘭水道", L"分岔口转进阿札克泥沼；踩压杆跨图；拉大压杆后回城", L"无"},
        {L"阿札克泥沼", L"【必拿】击杀BOSS沼泽女巫尹娜杜克+30精魂；回城传遗迹", L"【必拿】符文点1"},
        {L"淹沒之城", L"找到熔岩宝库开传送点后回城；跑图去污垢顶峰", L"无"},
        {L"熔岩寶庫", L"击杀BOSS冶炼师范梅克图；回城交任务解锁重铸台", L"无"},
        {L"污垢頂峰", L"击杀BOSS污垢女王；回城传送宝库", L"无"},
        {L"科佩克神殿", L"跑3层击杀BOSS艳阳神圣主教；坐电梯穿越时间进奥札尔", L"无"},
        {L"奧札爾", L"小怪掉牺牲之心；击杀BOSS邪魔毒蛇纳普阿兹；跑图去阿拉塔斯", L"无"},
        {L"暗霧殿堂", L"击杀关底BOSS结束第三章", L"无"}
    };
    chapterMap[3] = ch3;

    // 第四章
    ChapterData ch4;
    ch4.title = L"第四章 / Act 4";
    ch4.entries = {
        {L"金司馬區", L"主城：对话后船上找玛寇鲁，前往凯吉湾；间章后找老黑+2天赋", L"无"},
        {L"凱吉灣", L"岛1：终点处拾取地图碎片1", L"【必拿】地图碎片1"},
        {L"旅程之末", L"击杀哈特林船长；回城找丹尼克对话拿道具；杀BOSS交任务", L"无"},
        {L"瓦卡帕努島", L"岛2：找石化海盗得地图碎片2；击杀大白鲨得鱼鳍", L"【必拿】地图碎片2"},
        {L"吟謠洞窟", L"击杀呼唤之蚌换项链；击杀BOSS死亡之谣黛莫拉；和老黑对话回城", L"无"},
        {L"金氏島", L"岛3：【必拿】击杀眼盲巨兽+2天赋；沿海边找水于得地图碎片3", L"【必拿】地图碎片3"},
        {L"火山迷窟", L"击杀BOSS金氏领主克鲁托克；找旁边NPC对话后返回船上", L"无"},
        {L"伯勞鳥之島", L"岛5：击杀BOSS天之祸殃；巢穴里得地图碎片4；回城找玛寇鲁交碎片", L"【必拿】地图碎片4"},
        {L"悉妮蔻拉之眼", L"岛4：完成3个试炼到达沉默之厅；+5%最大魔力（岛2后开启）", L"无"},
        {L"亡者之殿", L"【必拿】通过3个试炼（元素抗or属性点）；击杀BOSS得先祖刺青+2天赋", L"无"},
        {L"祖靈的試煉", L"对话白之亚玛获得2点天赋，回城", L"无"},
        {L"掠奪者之角", L"岛6：藏宝图炸坟玩法；完成后解锁符文锻造系统", L"【必拿】最终解锁符文锻造"},
        {L"廢棄監獄", L"岛7：找到正义女神（30%药剂生命/魔力恢复二选一）", L"无"},
        {L"單獨禁閉室", L"击杀BOSS囚犯，回城", L"无"},
        {L"阿拉塔斯", L"岛8：到教堂攻击护罩，击杀BOSS救赎者之手托维安", L"无"},
        {L"挖掘", L"击杀BOSS第一使者本笃特斯；对话老黑回城", L"无"},
        {L"尼加卡努", L"岛9：跑图前进", L"无"},
        {L"部族之心", L"击杀BOSS酋长塔瓦凯；回城找老黑对话第四章结束", L"无"}
    };
    chapterMap[4] = ch4;

    // 间奏一
    ChapterData i1;
    i1.title = L"间奏一章 / Interlude A1（左线）";
    i1.entries = {
        {L"庇護所", L"营地对话；前往火噬农地", L"无"},
        {L"火噬農地", L"先找到黑木林记录点激活；击杀双BOSS黑白女巫；找到瑟雷之石入口", L"无"},
        {L"瑟雷之石", L"激活一圈符文巨石；回中间石板激活BOSS击杀；传送回火噬农地转黑木林", L"无"},
        {L"黑木林", L"沿对角线走，进入霍尔登", L"无"},
        {L"霍爾登", L"沿边缘走找狼之要塞进入；中间有NPC卖高阶符文", L"无"},
        {L"狼之要塞", L"【必拿】击杀BOSS恐惧典狱长，获得2点天赋；继续找霍尔登宅第", L"无"},
        {L"霍爾登宅第", L"击杀BOSS领主乌弗瑞克和艾尔斯威丝女士；间奏一结束", L"无"}
    };
    chapterMap[5] = i1;

    // 间奏二
    ChapterData i2;
    i2.title = L"间奏二章 / Interlude A2（中线）";
    i2.entries = {
        {L"卡里市集", L"营地对话；前往卡里交汇道", L"无"},
        {L"卡里交匯道", L"先找骨颚阶梯拿道具+5%最大生命；击杀BOSS沙虫和蟹子+2天赋；找卡塔尔之塘入口", L"无"},
        {L"卡塔爾之塘", L"绕大圈进入塞尔卡里庇护所；路遇贾莱关口先开传送点", L"无"},
        {L"塞爾卡里庇護所", L"小怪掉任务物品放两边台子换戒指项链珠宝；击杀BOSS眼镜蛇领主", L"无"},
        {L"賈萊關口", L"击杀BOSS堕炎；进入奇玛", L"无"},
        {L"奇瑪", L"沿斜对角走；找房间选属性加成；找奇玛水源地入口", L"无"},
        {L"奇瑪水源地", L"击杀BOSS法里登王子；间奏二结束", L"无"}
    };
    chapterMap[6] = i2;

    // 间奏三
    ChapterData i3;
    i3.title = L"间奏三章 / Interlude A3（右线）";
    i3.entries = {
        {L"林間空地", L"营地对话；前往灰烬森林", L"无"},
        {L"灰燼森林", L"绕U型走，找库莱亚村入口进入", L"无"},
        {L"庫萊亞村", L"【必拿】击杀BOSS莱塔拉+40精魂；进入冰川湖泊", L"无"},
        {L"冰川湖泊", L"沿边缘走；击杀BOSS拉卡尔进库莱亚山巅开传送点再回来", L"无"},
        {L"狂嗥洞穴", L"【必拿】击杀BOSS怪异血兽，获得2点天赋；回城交任务", L"无"},
        {L"庫萊亞山巔", L"沿路走；NPC领暗金装备（看脸）；找蚀刻溪谷入口", L"无"},
        {L"蝕刻溪谷", L"沿路直走关底击杀BOSS风暴血兽；进入库阿西克宝库", L"无"},
        {L"庫阿西克寶庫", L"击杀BOSS鲜血祭司；回城交任务；前往君峰镇找老黑+2天赋", L"无"}
    };
    chapterMap[7] = i3;

    for (int i = 1; i <= 7; i++) {
        for (const auto& entry : chapterMap[i].entries) {
            wstring guide = entry.storyReward + L"\n符文解锁：" + entry.seasonReward;
            guideMap[entry.mapName] = guide;
            guideMap[ToTraditional(entry.mapName)] = guide;
            guideMap[ToSimplified(entry.mapName)] = guide;
        }
    }
}

static wstring extractMapNameFromLog(const string& utf8Line) {
    size_t start = utf8Line.find("[SCENE] Set Source [");
    if (start == string::npos) return L"";
    start += 19;
    size_t end = utf8Line.find("]", start);
    if (end == string::npos) return L"";
    string rawMap = utf8Line.substr(start, end - start);
    if (rawMap.empty() || rawMap == "(null)") return L"";
    if (rawMap.front() == '[') rawMap.erase(0, 1);
    if (rawMap.back() == ']') rawMap.pop_back();
    wstring mapName = Utf8ToWString(rawMap);
    if (mapName.find(L"第") != wstring::npos && mapName.find(L"章") != wstring::npos) return L"";
    if (mapName.length() > 12 || mapName.length() < 2) return L"";
    return mapName;
}

static wstring getGuideByMapName(const wstring& mapName) {
    auto it = guideMap.find(mapName);
    if (it != guideMap.end()) return it->second;
    wstring simplified = ToSimplified(mapName);
    if (simplified != mapName) {
        it = guideMap.find(simplified);
        if (it != guideMap.end()) return it->second;
    }
    return L"暂无详细攻略\n探索发现吧！";
}

static void printChapter(int chapterId) {
    auto it = chapterMap.find(chapterId);
    if (it == chapterMap.end()) { cout << "无效章节" << endl; return; }
    const ChapterData& chapter = it->second;
    setGreen(); cout << WStringToGbk(L"\n========== " + chapter.title + L" 攻略 ==========") << endl; resetColor();
    cout << left << setw(6) << "序号"
        << setw(20) << "地图名"
        << setw(50) << "剧情内容及奖励"
        << setw(30) << "符文解锁" << endl;
    cout << string(106, '-') << endl;
    int idx = 1;
    for (const auto& e : chapter.entries) {
        cout << setw(6) << idx++;
        cout << setw(20) << WStringToGbk(e.mapName);
        if (e.storyReward.find(L"【必拿】") != wstring::npos) {
            setRed();
            cout << setw(50) << WStringToGbk(e.storyReward);
            resetColor();
        }
        else {
            cout << setw(50) << WStringToGbk(e.storyReward);
        }
        if (e.seasonReward.find(L"【必拿】") != wstring::npos || e.seasonReward.find(L"一定要拿") != wstring::npos) {
            setRed();
            cout << setw(30) << WStringToGbk(e.seasonReward);
            resetColor();
        }
        else {
            cout << setw(30) << WStringToGbk(e.seasonReward);
        }
        cout << endl;
    }
    setYellow(); cout << "提示：红色文字为【必拿】奖励" << endl; resetColor();
}

static void consoleMode() {
    while (true) {
        setGreen();
        cout << "\n========================================" << endl;
        cout << "      流放之路2 剧情攻略查询系统" << endl;
        cout << "========================================" << endl;
        resetColor();
        cout << "请输入章节号：1-7 (0返回): ";
        int choice; cin >> choice; cin.ignore();
        if (choice == 0) break;
        if (choice >= 1 && choice <= 7) printChapter(choice);
        else cout << "无效输入" << endl;
        cout << "\n按回车继续..."; cin.get();
    }
}

// ==================== 字体回退辅助函数 ====================
static HFONT CreateFallbackFont() {
    const wchar_t* fontNames[] = { L"Microsoft YaHei", L"SimHei", L"Segoe UI", L"Tahoma", L"Arial" };
    for (const wchar_t* fontName : fontNames) {
        HFONT hFont = CreateFontW(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH, fontName);
        if (hFont) return hFont;
    }
    return (HFONT)GetStockObject(DEFAULT_GUI_FONT);
}

// ==================== 悬浮窗窗口过程 ====================
static LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST: {
        POINT pt;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        ScreenToClient(hwnd, &pt);
        RECT rc;
        GetClientRect(hwnd, &rc);
        const int borderWidth = 8;
        if (pt.y < borderWidth) {
            if (pt.x < borderWidth) return HTTOPLEFT;
            if (pt.x > rc.right - borderWidth) return HTTOPRIGHT;
            return HTTOP;
        }
        if (pt.y > rc.bottom - borderWidth) {
            if (pt.x < borderWidth) return HTBOTTOMLEFT;
            if (pt.x > rc.right - borderWidth) return HTBOTTOMRIGHT;
            return HTBOTTOM;
        }
        if (pt.x < borderWidth) return HTLEFT;
        if (pt.x > rc.right - borderWidth) return HTRIGHT;
        return HTCLIENT;
    }
    case WM_SIZE: {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        g_windowWidth = rc.right - rc.left;
        g_windowHeight = rc.bottom - rc.top;
        SaveWindowConfig();
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_MOVE: {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        g_windowX = rc.left;
        g_windowY = rc.top;
        SaveWindowConfig();
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        HBRUSH bgBrush = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(hdc, &clientRect, bgBrush);
        DeleteObject(bgBrush);
        SetBkMode(hdc, TRANSPARENT);
        HFONT hFont = CreateFallbackFont();
        SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(255, 200, 0));
        RECT titleRect = { 10, 8, clientRect.right - 10, 38 };
        DrawTextW(hdc, L"流放之路2 - 剧情攻略", -1, &titleRect, DT_LEFT);
        SetTextColor(hdc, RGB(0, 255, 100));
        RECT mapRect = { 10, 38, clientRect.right - 10, 68 };
        DrawTextW(hdc, (L"当前地图：" + g_currentMap).c_str(), -1, &mapRect, DT_LEFT);
        RECT guideRect = { 10, 68, clientRect.right - 10, clientRect.bottom - 8 };
        if (g_currentGuide.find(L"【必拿】") != wstring::npos)
            SetTextColor(hdc, RGB(255, 100, 100));
        else
            SetTextColor(hdc, RGB(220, 220, 220));
        DrawTextW(hdc, g_currentGuide.c_str(), -1, &guideRect, DT_LEFT | DT_WORDBREAK);
        DeleteObject(hFont);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_LBUTTONDOWN: {
        POINT pt;
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        if (pt.y < 38) {
            SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

static void createOverlayWindow() {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"POE2OverlayClass";
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassExW(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    LoadWindowConfig();
    int x, y;
    if (g_windowX >= 0 && g_windowY >= 0) {
        x = g_windowX;
        y = g_windowY;
    }
    else {
        x = screenWidth - g_windowWidth - 20;
        y = screenHeight - g_windowHeight - 80;
    }
    g_overlayHwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"POE2OverlayClass",
        L"POE2攻略悬浮窗",
        WS_POPUP | WS_THICKFRAME,
        x, y, g_windowWidth, g_windowHeight,
        NULL, NULL, GetModuleHandleW(NULL), NULL);
    SetLayeredWindowAttributes(g_overlayHwnd, RGB(0, 0, 0), 200, LWA_COLORKEY | LWA_ALPHA);
    ShowWindow(g_overlayHwnd, SW_SHOW);
    UpdateWindow(g_overlayHwnd);
}

static DWORD WINAPI LogMonitorThread(LPVOID param) {
    SetConsoleOutputCP(936);
    SetConsoleCP(936);
    ifstream logFile;
    DWORD lastSize = 0;
    wstring lastMap;
    while (g_overlayMode) {
        if (!fileExists(g_gameLogPath)) { Sleep(1000); continue; }
        logFile.open(g_gameLogPath, ios::binary);
        if (!logFile.is_open()) { Sleep(500); continue; }
        logFile.seekg(0, ios::end);
        DWORD curSize = (DWORD)logFile.tellg();
        if (curSize != lastSize && curSize > 0) {
            long long readPos = max(0LL, (long long)curSize - 16384);
            logFile.seekg(readPos);
            string buf;
            buf.resize(curSize - readPos);
            logFile.read(&buf[0], buf.size());
            logFile.close();
            vector<wstring> maps;
            size_t ls = 0;
            for (size_t i = 0; i < buf.size(); i++) {
                if (buf[i] == '\n') {
                    string line = buf.substr(ls, i - ls);
                    ls = i + 1;
                    wstring name = extractMapNameFromLog(line);
                    if (!name.empty()) maps.push_back(name);
                }
            }
            if (!maps.empty()) {
                wstring name = maps.back();
                if (name != lastMap) {
                    lastMap = name;
                    g_currentMap = name;
                    g_currentGuide = getGuideByMapName(name);
                    if (g_overlayHwnd) {
                        InvalidateRect(g_overlayHwnd, NULL, TRUE);
                        UpdateWindow(g_overlayHwnd);
                    }
                    setYellow();
                    cout << "\n[检测到新地图] " << WStringToGbk(name) << endl;
                    resetColor();
                }
            }
            lastSize = curSize;
        }
        else {
            logFile.close();
        }
        Sleep(500);
    }
    return 0;
}

static void overlayMode() {
    g_overlayMode = true;
    g_gameLogPath = getLogPath();
    if (g_gameLogPath.empty()) {
        setRed();
        cout << "\n[错误] 未能获取有效的日志路径，已返回主菜单。" << endl;
        resetColor();
        system("pause");
        return;
    }
    createOverlayWindow();
    setGreen();
    cout << "\n[OK] 监控: " << WStringToGbk(g_gameLogPath) << endl;
    cout << "[OK] 悬浮窗已创建，可拖拽边框调整大小" << endl;
    resetColor();
    g_monitorThread = CreateThread(NULL, 0, LogMonitorThread, NULL, 0, NULL);
    MSG msg;
    while (g_overlayMode && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (g_monitorThread) {
        TerminateThread(g_monitorThread, 0);
        CloseHandle(g_monitorThread);
    }
}

int main() {
    SetConsoleOutputCP(936);
    SetConsoleCP(936);
    initializeChapters();
    while (true) {
        setGreen();
        cout << "\n====================================================" << endl;
        cout << "      流放之路2 剧情攻略查询系统 [国际服]" << endl;
        cout << "      本工具为开源/免费，仅供学习交流" << endl;
        cout << "      绿色工具,有账号风险顾虑立即删除本软件" << endl;
        cout << "      工作原理:读取日志>判断所在地图>给出对应攻略" << endl;
        cout << "====================================================" << endl;
        resetColor();
        cout << "1 - 手动查询模式（输入章节号）" << endl;
        cout << "2 - 悬浮窗模式（自动识别地图）" << endl;
        setYellow();
        cout << "0 - 退出" << endl;
        cout << "\n[彩蛋] 输入 \"洋葱真帅\" 有惊喜" << endl;
        resetColor();
        cout << "====================================================" << endl;
        cout << "请选择: ";
        string input;
        getline(cin, input);
        if (input == "1") consoleMode();
        else if (input == "2") overlayMode();
        else if (input == "0") break;
        else if (input == "洋葱真帅") {
            setRed();
            cout << "\n[惊喜] 恭喜你发现了彩蛋！" << endl;
            cout << "正在为你打开流放之路2 工具导航站..." << endl;
            resetColor();
            OpenUrl(L"https://poe.onions.top");
            system("pause");
        }
        else {
            cout << "无效输入，请重新选择。" << endl;
            system("pause");
        }
    }
    cout << "再见！" << endl;
    return 0;
}