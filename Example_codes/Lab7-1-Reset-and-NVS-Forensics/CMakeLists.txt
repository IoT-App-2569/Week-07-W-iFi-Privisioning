cmake_minimum_required(VERSION 3.16)

# ป้องกันไม่ให้ IDF ดึง component เก่า wifi_provisioning มาชนกับ
# network_provisioning ที่ประกาศไว้ใน main/idf_component.yml (ESP-IDF 6.0+)
set(EXCLUDE_COMPONENTS wifi_provisioning)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(lab7_1_reset_nvs_forensics)