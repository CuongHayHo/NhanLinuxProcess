#!/bin/sh
showDateTime() {
  clear
  echo "Thời gian hiện tại: $(date +%Y-%m-%d\ %H:%M:%S)"
  showMenu
}

updateDateTimeManual() {
  clear
  echo "--- Thiết lập thời gian hệ thống ---"
  echo "Thời gian hiện tại: $(date +%Y-%m-%d\ %H:%M:%S)"
  read -p "Nhập thời gian mới (định dạng YYYY-MM-DD HH:mm:ss, vd: 2024-03-16 09:18:01): " newDateTime
  timedatectl set-time "$newDateTime"
  echo "Thời gian đã được cập nhật: $(date +%Y-%m-%d\ %H:%M:%S)"
  showMenu
}

updateDateTimeAuto() {
  clear
  date +%Y-%m-%d\ %H:%M:%S -s "$(wget -qSO- --max-redirect=0 google.com 2>&1 | grep Date: | cut -d' ' -f5-8)Z" |
  if  $? -eq 0 ]; then
    echo "Thời gian đã được thiết lập tự động"
  else
    echo "Không thể thiết lập thời gian tự động, hãy thử lại sau"
  fi
  
  
  showMenu
}

showMenu() {
  echo
  echo
  echo
  echo "--- Thiết lập thời gian hệ thống ---"
  echo "1. Thời gian hiện tại"
  echo "2. Thiết lập thời gian thủ công"
  echo "3. Thiết lập thời gian tự động"
  echo "0. Thoát"
  read -p "Lựa chọn: " option

  case $option in
    1)
      showDateTime
      ;;
    2)
      updateDateTimeManual
      ;;
    3)
      updateDateTimeAuto
      ;;
    0)
      exit 0
      ;;
    *)
      clear
      echo "Vui lòng nhập đúng lựa chọn"
      showMenu
      ;;
  esac
}

showMenu