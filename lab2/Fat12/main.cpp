
#include <fstream>
#include <iostream>
#include <cstring>
#include <assert.h>
#include <regex>


#pragma pack(push)
#pragma pack(1)

#define FAT_START  0x0200                // FAT12中，FAT区从第1扇区开始，1 * 512 = 512 = 0x0200
#define ROOT_START 0x2600                // FAT12中，根目录区从第19扇区开始，19 * 512 = 9728 = 0x2600
#define DATA_START 0x4200               // FAT12中，数据区从第33扇区开始，33 * 512 = 16896 = 0x4200


char path[] =  "/home/cyc/apps/clion/Fat12/a.img"; //
FILE *fat12;
enum {
    DIR_TYPE,
    FILE_TYPE,
    END_DIR_TYPE,
};

enum {
    COMMAND_ERROR, // 没有这个命令
    PATH_ERROR,     // 找不到相关路径或文件
    PARM_ERROR,     // 参数错误
    PARM_MISS,      // 缺少参数
};
using namespace  std;
string output;
struct Fat12Header{
    char BS_OEMName[8];    // OEM字符串，必须为8个字符，不足以空格填空
    ushort BPB_BytsPerSec; // 每扇区字节数
    u_char BPB_SecPerClus;  // 每簇占用的扇区数
    ushort BPB_RsvdSecCnt; // Boot占用的扇区数
    u_char BPB_NumFATs;     // FAT表的记录数
    ushort BPB_RootEntCnt; // 最大根目录文件数
    ushort BPB_TotSec16;   // 每个FAT占用扇区数
    u_char BPB_Media;       // 媒体描述符
    ushort BPB_FATSz16;    // 每个FAT占用扇区数
    ushort BPB_SecPerTrk;  // 每个磁道扇区数
    ushort BPB_NumHeads;   // 磁头数
    uint BPB_HiddSec;      // 隐藏扇区数
    uint BPB_TotSec32;     // 如果BPB_TotSec16是0，则在这里记录
    u_char BS_DrvNum;       // 中断13的驱动器号
    u_char BS_Reserved1;    // 未使用
    u_char BS_BootSig;      // 扩展引导标志
    uint BS_VolID;         // 卷序列号
    char BS_VolLab[11];    // 卷标，必须是11个字符，不足以空格填充
    char BS_FileSysType[8];// 文件系统类型，必须是8个字符，不足填充空格
};

Fat12Header fat12Header;
/**
 * RootEntry：根目录项
 */
struct RootEntry{
    u_char DIR_Name[11];  //文件名
    u_char DIR_Attr;     // 文件属性    0x10 directory 0x20 achieve
    u_char reserve[10];  // 保留位
    ushort DIR_WrtTime;  // 最后写入时间
    ushort DIR_WrtDate;  // 写入日期
    ushort DIR_FstClus; // 文件开始簇号
    uint DIR_FileSize;  // 文件大小
};

/**
 * listNode: 文件、目录节点
  */
struct ListNode{
    char name[11]; //文件名
    char path[64]; //文件路径、文件全名
    int type; //类型： 0: 文件 1：// 文件夹
    int startpos; //文件起始位置，目录为0
    int subpos; //子文件位置 如果没有子文件则为-1
    int filenum; // 子文件数
    int dirnum; //子目录数
    uint size;
};

void initList();

int input_cd();

void cat_(string *inputs, int num);

void ls_(string *inputs, int num);

void ls_(string p, bool is_l);

void error(int error_type);

void node_print(ListNode *pNode);

void file_print(int pos);

int getNextPos(int pos, int start);

string stdFileName(string name);

string stdDirName(string name);

void node_detail_print(ListNode *pNode);

extern "C" {void sprint(const char *output, int len);}

int listLen;

ListNode lists[100];

void InitHeader(Fat12Header& fat12Header, string p){
    ifstream ifile;
    ifile.open(p, ios::in);

    if (!ifile.is_open()){
        cout << "Error: opening file fail"<< endl;
        exit(1);
    }

    ifile.seekg(3);
    ifile >> fat12Header.BS_OEMName;
    ifile.seekg(11);
    ifile >> fat12Header.BPB_BytsPerSec;
    ifile.seekg(13);
    ifile >> fat12Header.BPB_SecPerClus;
    ifile.seekg(17);
    ifile >> fat12Header.BPB_RootEntCnt;

    ifile.close();
    fat12Header.BPB_BytsPerSec = 0x200;

    cout << "BPB_BytsPerSec: " << hex << (fat12Header.BPB_BytsPerSec) << endl;
    cout << "BPB_SecPerClus: " << to_string(fat12Header.BPB_SecPerClus)<< endl;
    cout << "BPB_RootEntCnt: " << hex << fat12Header.BPB_RootEntCnt << endl;

}

RootEntry FindRootEntry(int start, int i)
{
    struct RootEntry ret;
    struct RootEntry *entry_ptr = &ret;

    ifstream ifile;
    ifile.open(path, ios::in);

    // BPB_RootEntCnt：目录项个数
    if( ifile.is_open() && (0 <= i) && (i < fat12Header.BPB_RootEntCnt) )
    {

        // 定位到第i个目录项
        ifile.seekg(start + i * sizeof(RootEntry));

//        in.readRawData(reinterpret_cast<char*>(&ret), sizeof(ret));
        ifile
        >>  ret.DIR_Name
        >>  ret.DIR_Attr
        >>  ret.reserve
        >>  ret.DIR_WrtTime
        >>  ret.DIR_WrtDate
        >>  ret.DIR_FstClus
        >>  ret.DIR_FileSize;

    }

    ifile.close();

    return ret;
}

void FindRootEntry(Fat12Header& fat12Header, const char *p){
    struct RootEntry entry;
    struct RootEntry *entry_ptr = &entry;
    for(int i = 0; i < fat12Header.BPB_RootEntCnt; i++){
        entry = FindRootEntry(ROOT_START, i);
        if (entry.DIR_Name[0] != '\0'){
            cout << i <<". ";
            cout << entry.DIR_Name << endl;
        }
        else{
            break;
        }
    }
}
/**
 * 读取文件，将文件加载进链表
 * 读取目录
 * @param parentNode 父节点（上级目录）
 * @param start 起始读取位置
 * */
void loadFileToList(int parentNode, int start){
    ListNode *parent_node = &lists[parentNode];
    parent_node->subpos = listLen;     // 当前节点为上级目录的第一个子节点
    parent_node->filenum = 0;
    parent_node->dirnum = 0;

    RootEntry entry;
    RootEntry *entry_p = &entry;

    string s = "/";
    // 一个簇512B， 一个目录项32B， 512/32=16
    for (int i = 0; i < 16; i++) {
         fseek(fat12, i*32+start, SEEK_SET);
         fread(entry_p, 1, 32, fat12);
        //entry = (FindRootEntry(start, i));
        ListNode *cur_node = &lists[listLen]; // cur_node
        //识别到空目录项
        if ((entry_p->DIR_Name[0] == 0x00) || (entry_p->DIR_Name[0] == 0xe5)){
            break;
        }
        if (entry_p->DIR_Attr == 0x20 || entry_p->DIR_Attr == 0x10){
            listLen++;

            int pathLen = strlen(lists[parentNode].path);
            int nameLen = 11;

            memcpy(cur_node->name, entry_p->DIR_Name, nameLen);         // 文件名
            cur_node->name[11] = 0;// 终止符
            // 文件
            if (entry_p->DIR_Attr == 0x20){
                cur_node->type = FILE_TYPE;
                cur_node->subpos = -1;
                parent_node->filenum++;
                string std_name = stdFileName(cur_node->name);
                strcpy(cur_node->name, std_name.c_str());
            }
            // 目录
            else if(entry_p->DIR_Attr == 0x10){
                if (entry_p->DIR_Name[0] == '.'){
                    cur_node->type = END_DIR_TYPE;
                } else{
                    cur_node->type = DIR_TYPE;
                }
                parent_node->dirnum++;
                string std_name = stdDirName(cur_node->name);
                strcpy(cur_node->name, std_name.c_str());
            }
            cur_node->name[10] = 0;// 终止符

            memcpy(cur_node->path, lists[parentNode].path, pathLen);
            memcpy(cur_node->path+pathLen, s.c_str(), s.length());
            memcpy(cur_node->path+pathLen+s.length(), cur_node->name, nameLen); // 路径 =  父目录路径 + / + 文件名
            cur_node->startpos = entry_p->DIR_FstClus;      // 开始簇号
            cur_node->size = entry_p->DIR_FileSize;
            cur_node->filenum = 0;
            cur_node->dirnum = 0;
        }
    }

    int son_num = parent_node->dirnum + parent_node->filenum;
    int first_son = parent_node->subpos;
    for (int i = 0; i < son_num; ++i) {
        ListNode *cur_node = &lists[first_son+i];
        // 递归读子目录的目录项
        if (cur_node->type == DIR_TYPE){
            int start_add = DATA_START + 512*(cur_node->startpos-2);
            loadFileToList(first_son+i, start_add);
        }
    }


}

/**
 * 读取fat12文件
 * */
void readFile(){
    fat12 = fopen(path, "rb"); //以读模式打开镜像文件

    listLen = 0;
    initList();

    if (fat12 == NULL){
        cout << "Error: file not found\n";
    } else{
        loadFileToList(0, ROOT_START);
        lists[0].path[0] = '/';
    }

    fclose(fat12);
}

void initList() {
    lists[listLen].name[0] = '/';
//    lists[listLen].path[0] = '/';
    lists[listLen].dirnum = 0;
    lists[listLen].filenum = 0;
    lists[listLen].startpos = DATA_START;
    lists[listLen].subpos = 2;
    lists[listLen].type = DIR_TYPE;
    listLen++;
}


int main() {
    std::cout << "Hello, World!" << std::endl;

    InitHeader(fat12Header, path);

    readFile();

    // FindRootEntry(fat12Header, path);

    int flag = 1;
    while(flag){
        flag = input_cd();
        sprint(output.c_str(), output.length());
        // cout << output;
        output = "";
    }

    return 0;
}


int input_cd() {
//    string input;
//    cin >> input;

    int ret = 1;
    string inputs[10];
    int num = 0;

    cin >> inputs[num];
    num++;
    while (cin.get() != '\n'){
        cin >> inputs[num];
        num++;
    }

    if (inputs[0] == "exit"){
        ret = 0;
    }
    else if (inputs[0] == "ls"){
        ls_(inputs, num);
    }
    else if (inputs[0] == "cat"){
        cat_(inputs, num);
    }
    else{
        error(COMMAND_ERROR);
    }
    return ret;

}

void error(int error_type) {
    switch (error_type) {
        case COMMAND_ERROR:
            output += "没有这个命令\n";
            break;
        case PARM_ERROR:
            output += "参数错误\n";
            break;
        case PATH_ERROR:
            output += "找不到此目录或文件\n";
            break;
        case PARM_MISS:
            output += "缺少参数\n";
            break;
        default: ;
    }
}
void ls_(string* inputs, int num) {

    regex regex1("^(-)l+");
    regex regex2("-.*");
    string parm = "/";    // 路径，默认路径为“/”
    bool is_l = false;   // 是否有-l参数
    int num_path = 0;

    if (num == 1){  // ls
    }
    else if (num == 2){ // ls -l || ls dir
        if (regex_match(inputs[1], regex1)){is_l = true;}
        else if (regex_match(inputs[1], regex2)&& !(regex_match(inputs[1], regex1))){num_path = 2;}
        else{parm = inputs[1];}
    }
    else if (num > 2){ // ls -l dir || error
        is_l = true;
        for (int i = 1; i < num; ++i) { // 记录格式不为regex1的参数，这些参数即为要ls的路径，如果路径个数大于1，则说明命令有误
            if (!regex_match(inputs[i], regex1)){
                parm = inputs[i];
                num_path++;
            }
        }
    }


    if (num_path > 1){
        error(PARM_ERROR);
    }
    else{
        ls_(parm, is_l);
    }

}

void ls_(string p, bool is_l) {

    ListNode* cur_node;
    int i = 0;
    for (i; i < listLen; ++i) {
        cur_node = &lists[i];
        if ((cur_node->name == p && p == "/") ||( cur_node->path == p && cur_node->type == DIR_TYPE)){
            break;
        }
    }

    if (i == listLen){
        error(PATH_ERROR);
    }
    else{
        if (is_l){
            node_detail_print(cur_node);
        }
        else{
            node_print(cur_node);
        }
    }
}


void node_print(ListNode *pNode) {
    int start = pNode->subpos;  // 父节点的第一个子节点位置， 顺序往后遍历
    output += pNode->path;
    output += ':';
    output += '\n';
    ListNode *cur_node;
    int son_num = pNode->dirnum+pNode->filenum;
    // 子目录和子文件的名称
    for (int i = 0; i < son_num; ++i) {
        cur_node = &lists[start+i];
        if (cur_node->type == DIR_TYPE || cur_node->type == END_DIR_TYPE){
            output = output + "\033[31m" + cur_node->name + "\033[0m";
        } else output += cur_node->name;
        output += ' ';
        output += ' ';
    }
    output += '\n';
    // 对子目录递归
    for (int i = 0; i < son_num; ++i) {
        cur_node = &lists[start+i];
        if ((cur_node->dirnum+cur_node->filenum) != 0){
            node_print(cur_node);
        }
    }
}

void node_detail_print(ListNode *pNode){
    int start = pNode->subpos;  // 父节点的第一个子节点位置， 顺序往后遍历
    // 输出当前父目录的信息
    output += pNode->path;
    output += ' ';
    output += to_string(pNode->dirnum);
    output += ' ';
    output += to_string(pNode->filenum);
    output += ':';
    output += '\n';

    ListNode *cur_node;
    int son_num = pNode->dirnum+pNode->filenum;
    // 子目录和子文件的名称
    for (int i = 0; i < son_num; ++i) {
        cur_node = &lists[start+i];

        if (cur_node->type == DIR_TYPE || cur_node->type == END_DIR_TYPE){
            output = output + "\033[31m" + cur_node->name + "\033[0m";
        } else output += cur_node->name;

        if (cur_node->type == DIR_TYPE){
            output = output + ' ' + to_string(cur_node->dirnum-2) + ' ' + to_string(cur_node->filenum);
        } else if (cur_node->type == FILE_TYPE){
            output = output + ' ' + to_string(cur_node->size);
        }
        output += '\n';
    }
    for (int i = 0; i < son_num; ++i) {
        cur_node = &lists[start+i];
        if ((cur_node->dirnum+cur_node->filenum) != 0){
            node_detail_print(cur_node);
        } else{
            break;
        }
    }
}

void cat_(string *inputs, int num) {
    string parm = inputs[1];
    int i = 0;
    ListNode *listNode;
    for (i; i < listLen; ++i) {
        listNode = &lists[i];
        if (listNode->path == parm && listNode->type == FILE_TYPE){
            break;
        }
    }
    int pos = listNode->startpos;

    if (num > 2){
        error(PARM_ERROR);
    } else if (num < 2){
        error(PARM_MISS);
    }
    else if(num == 2){
        if (i == listLen){
            error(PATH_ERROR);
        } else{
            fat12 = fopen(path, "r");
            file_print(pos);
            fclose(fat12);
        }
    }

}

/**
 *
 * @param pos 簇号 pos>=2
 * 数据区的簇号是从2开始的
 * */
void file_print(int pos){
    assert(pos >= 2);
    int off = DATA_START+(pos-2)*512;
    fseek(fat12, off, SEEK_SET); // 偏移：数据区开始+（簇号-2）*512

    char file;
    char *file_p = &file;

    for (int i = 0; i < 512; ++i) {
        fread(file_p, 1, 1, fat12);
        if (file == 0){
            break;
        }
        output += file;
        off++;
        fseek(fat12, off, SEEK_SET);
    }



    int nextPos = getNextPos(pos, FAT_START); //
    if (nextPos >= 0x0ff8){
        // 最后一个簇

    } else if(nextPos == 0x0ff7){
        // 坏簇

    } else{
        // 有下一个簇
        file_print(nextPos);
    }

}
/**
 *
 * @param pos 当前簇号
 * 根据pos， 去fat表中找位置对应的fat项，fat项 12bit =
 * */
int getNextPos(int pos, int start) {
    int nextPos = 0;

    int flag = pos % 2;     //  flag: 读取3个字节 0 右移12位；1 左12位置0
    int off = start+(pos/2)*3; // 偏移量

    int* pInt = &nextPos;
    fseek(fat12, off, SEEK_SET);
    fread(pInt, 1, 3, fat12);
    if (flag == 1){
        nextPos = nextPos >> 12;      //右移12位（取左12位）
    } else if(flag == 0){
        nextPos = nextPos & 0x0fff; //取右12位
    }
    return nextPos;

}

string stdFileName(string name){
    int n = name.length();
    int i = 0;
    for (i; i < n; i++) {
        if (name[i] == ' '){
            name[i] = '.';
            break;
        }
    }
    i++;
    for (i; i < n; ++i) {
        if (name[i] == ' '){
            char temp = ' ';
            for (int j = i; j < n-1; ++j) {
                name[j] = name[j+1];
            }
            name[name.length()-1] = 0;
            i--;
        } else{
            break;
        }
    }
    return name;
}

string stdDirName(string name){
    int n = name.length();
    int i = 0;
    for (i; i < n; i++) {
        if (name[i] == ' '){
            name[i] = 0;
            break;
        }
    }
    return name;
}