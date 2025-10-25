#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
  
constexpr const char* thermalPath = "/sys/class/thermal";
  
bool checkSensorPath(const char *type_str) {
    return strstr(type_str, "soc_max") != nullptr || strstr(type_str, "mtktscpu") != nullptr ||strstr(type_str, "cpu-1-") != nullptr;
}
  
int openZonePath(const char* zoneName){
    char path[256];
    snprintf(path, sizeof(path), "%s/%s/type", thermalPath, zoneName);

    auto fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    char buf[64];
    auto n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    
    if (n <= 0) return -1;
    
    buf[n - 1] = '\0';
    
    if (!checkSensorPath(buf)) {
        close(fd);
        return -1;
    }

    char temp[256];
    snprintf(temp, sizeof(temp), "%s/%s/temp", thermalPath, zoneName);
    return open(temp, O_RDONLY);
}
  
int readTemp(int fd) {
    char buf[32];
    lseek(fd, 0, SEEK_SET);
    auto n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n <= 0) return -1;
    buf[n - 1] = '\0';
    return atoi(buf);
}
  
int getMaxCpuTemp() {
    int temp = -1;
    auto dir = opendir(thermalPath);

    if (dir == nullptr) {
        printf("无法打开文件夹:%s\n", thermalPath);
        return -1;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != nullptr){
        if (strncmp(entry->d_name,"thermal_zone", 12) != 0) continue;    
        auto fd = openZonePath(entry->d_name);
        if (fd < 0) continue;
        temp = readTemp(fd);
        close(fd);   
    }

    closedir(dir);
    return temp / 1000.0; // 舍去部分小数
}
  
int main(void) {
    auto temp = getMaxCpuTemp();
      
    if (temp < 0) { printf("未检测到温度传感器\n"); return 1; }

    printf("CPU温度: %d °C\n", temp);
}