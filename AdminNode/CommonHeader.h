#pragma once
#include <DarkNebula/DarkNebulaGlobal.h>

constexpr auto IniFilename = "Admin.ini";
constexpr auto IniAdminSendPort = "/Admin/SendPort";
constexpr auto IniAdminSendPortDefault = dn::ADMIN_SEND_PORT;
constexpr auto IniAdminRecvPort = "/Admin/RecvPort";
constexpr auto IniAdminRecvPortDefault = dn::ADMIN_RECEIVE_PORT;
constexpr auto IniSimTime = "/Admin/SimTime";
constexpr auto IniSimTimeDefault = 20;
constexpr auto IniSimStep = "/Admin/SimStep";
constexpr auto IniSimStepDefault = 10;
constexpr auto IniSimSpeed = "/Admin/SimSpeed";
constexpr auto IniSimSpeedDefault = 100;
constexpr auto IniFreeSim = "/Admin/FreeSim";
constexpr auto IniFreeSimDefault = false;