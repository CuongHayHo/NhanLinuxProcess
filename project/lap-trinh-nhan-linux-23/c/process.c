#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

void listProcess() {
  system("top");
}

void killProcess() {
  int pid;

  system("clear");
  printf("--- Quản lí tiến trình - Dừng tiến trình ---\n");
  printf("Nhập PID của tiến trình muốn dừng: ");
  scanf("%d", &pid);
  if(kill(pid, 9) == 0) printf("Đã dừng tiến trình %d.\n", pid);
  else printf("Không thể dừng tiến trình %d.\n", pid);
}

void showMenu() {
  int option = -1;

  printf("\n\n\n");
  printf("--- Quản lí tiến trình ---\n");
  printf("1. Danh sách tiến trình\n");
  printf("2. Dừng tiến trình\n");
  printf("0. Thoát\n");
  
  while(option < 0 || option > 4) {
    printf("Lựa chọn: ");
    scanf("%d", &option);
  }

  switch (option) {
    case 1:
      listProcess();
      break;
    case 2:
      killProcess();
      break;
    default:
      exit(0);
  }
}

void main() {
  while (true) {
    showMenu();
  }
}