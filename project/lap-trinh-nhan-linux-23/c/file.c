#include <stdio.h>
#include <stdlib.h>

void createFile() {
  FILE *file = NULL;
  char fileName[50];
  
  system("clear");
  printf("--- Quản lí file - Tạo file ---\n");
  printf("Nhập tên file: ");
  scanf("%s", fileName);
  file = fopen(fileName, "w");
  if(file) {
    printf("Tạo file '%s' thành công.\n", fileName);
    fclose(file);
  }
  else printf("Không thể tạo file '%s'.\n", fileName);
}

void readFile() {
  FILE *file = NULL;
  char fileName[50];
  
  system("clear");
  printf("--- Quản lí file - Đọc file ---\n");
  printf("Nhập tên file: ");
  scanf("%s", fileName);
  file = fopen(fileName, "r");
  if(file) {
    char fileContent[100];
    while(fgets(fileContent, sizeof(fileContent), file))
      printf("%s", fileContent);
    printf("\n");
    fclose(file);
  }
  else printf("Không thể đọc file '%s'.\n", fileName);
}

void deleteFile() {
  FILE *file = NULL;
  char fileName[50];
  
  system("clear");
  printf("--- Quản lí file - Xoá file ---\n");
  printf("Nhập tên file: ");
  scanf("%s", fileName);
  file = fopen(fileName, "r");
  if(file) {
    fclose(file);
    if (remove(fileName) == 0)
      printf("Xoá file '%s' thành công.\n", fileName);
    else
      printf("Không thể xoá file '%s'.\n", fileName);
  }
  else printf("File '%s' không tồn tại.\n", fileName);
}

void showMenu() {
  int option;

  printf("\n\n\n");
  printf("--- Quản lí file ---\n");
  printf("1. Tạo file\n");
  printf("2. Đọc file\n");
  printf("3. Xoá file\n");
  printf("0. Thoát\n");
  
  while(option < 0 || option > 4) {
    printf("Lựa chọn: ");
    scanf("%d", &option);
  }

  switch (option) {
    case 1:
      createFile();
      break;
    case 2:
      readFile();
      break;
    case 3:
      deleteFile();
      break;
    default:
      exit(0);
  }
}



void main() {
  showMenu();
}