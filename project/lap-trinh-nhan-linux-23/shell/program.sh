
installProgram() {
  clear
  echo "--- Quản lý chương trình - Cài đặt chương trình ---"
  read -p "Nhập tên chương trình muốn cài: " program_name
  if dpkg -l | grep -q "^ii  $program_name "; then
    echo "Chương trình '$program_name' đã được cài đặt."
  else
    echo "Chương trình '$program_name' chưa được cài đặt. Đang cài đặt..."
    sudo apt install "$program_name"
    if dpkg -l | grep -q "^ii  $program_name "; then
      echo "Cài đặt chương trình '$program_name' thành công."
    else
      echo "Cài đặt chương trình '$program_name' thất bại."
    fi
  fi

  showMenu
}

uninstallProgram() {
  clear
  echo "--- Quản lý chương trình - Gỡ bỏ chương trình ---"
  read -p "Nhập tên chương trình muốn gỡ bỏ: " program_name
  if dpkg -l | grep -q "^ii  $program_name "; then
    echo "Tìm thấy chương trình '$program_name'. Đang gỡ bỏ ..."
    sudo apt remove "$program_name"
    if dpkg -l | grep -q "^ii  $program_name "; then
      echo "Gỡ bỏ chương trình '$program_name' thất bại."
    else
      echo "Gỡ bỏ chương trình '$program_name' thành công."
    fi
  else
    echo "Chương trình '$program_name' chưa được cài đặt."
    
  fi
    showMenu
}

listPrograms() {
    clear
    echo "--- Quản lý chương trình - Danh sách chương trình ---"
    dpkg --get-selections | grep -v deinstall
    showMenu
}

listProgramsCanInstall() {
    clear
    echo "--- Quản lý chương trình - Danh sách chương trình có thể cài đặt---"
    apt list
    showMenu
}


showMenu() {
  echo
  echo
  echo
  echo "--- Quản lý chương trình ---"
  echo "1. Cài đặt chương trình"
  echo "2. Gỡ bỏ chương trình"
  echo "3. Danh sách chương trình"
  echo "4. Danh sách chương trình có thể cài đặt"
  echo "0. Thoát"
  read -p "Lựa chọn: " option
  
  case $option in
    1)
      installProgram
      ;;
    2)
      uninstallProgram
      ;;
    3)
      listPrograms
      ;;
    4)
      listProgramsCanInstall
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