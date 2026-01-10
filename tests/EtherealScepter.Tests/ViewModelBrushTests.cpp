//Created by: EternityBoundary
// Unit tests for DashboardViewModel brush logic
#include "pch.h"

enum class TestColor
{
    Green,
    Yellow,
    Orange,
    Red,
    Gray,
    LightGreen
};

namespace DashboardBrushLogic
{
    TestColor GetNetworkStatusColor(const std::wstring& networkStatus)
    {
        if (networkStatus == L"Internet")
            return TestColor::Green;
        if (networkStatus == L"Local Network")
            return TestColor::Yellow;
        if (networkStatus == L"Disconnected")
            return TestColor::Red;
        return TestColor::Gray;
    }

    TestColor GetCgnatStatusColor(const std::wstring& cgnatStatus)
    {
        if (cgnatStatus == L"CGNAT")
            return TestColor::Red;
        if (cgnatStatus == L"Open")
            return TestColor::Green;
        return TestColor::Gray;
    }

    TestColor GetUpnpStatusColor(const std::wstring& upnpStatus)
    {
        if (upnpStatus == L"Enabled")
            return TestColor::Green;
        if (upnpStatus == L"Disabled")
            return TestColor::Orange;
        return TestColor::Gray;
    }

    TestColor GetNatTypeColor(const std::wstring& natType)
    {
        if (natType == L"Open")
            return TestColor::Green;
        if (natType == L"Moderate")
            return TestColor::Orange;
        if (natType == L"Strict")
            return TestColor::Red;
        return TestColor::Gray;
    }

    TestColor GetStatusColor(bool hasError)
    {
        if (hasError)
            return TestColor::Red;
        return TestColor::LightGreen;
    }
}

// ===========================================
// NetworkStatusBrush Tests
// ===========================================

TEST(DashboardViewModelBrushTests, NetworkStatus_Internet_ReturnsGreen)
{
    auto result = DashboardBrushLogic::GetNetworkStatusColor(L"Internet");
    EXPECT_EQ(TestColor::Green, result);
}

TEST(DashboardViewModelBrushTests, NetworkStatus_LocalNetwork_ReturnsYellow)
{
    auto result = DashboardBrushLogic::GetNetworkStatusColor(L"Local Network");
    EXPECT_EQ(TestColor::Yellow, result);
}

TEST(DashboardViewModelBrushTests, NetworkStatus_Disconnected_ReturnsRed)
{
    auto result = DashboardBrushLogic::GetNetworkStatusColor(L"Disconnected");
    EXPECT_EQ(TestColor::Red, result);
}

TEST(DashboardViewModelBrushTests, NetworkStatus_Unknown_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetNetworkStatusColor(L"Unknown");
    EXPECT_EQ(TestColor::Gray, result);
}

TEST(DashboardViewModelBrushTests, NetworkStatus_Empty_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetNetworkStatusColor(L"");
    EXPECT_EQ(TestColor::Gray, result);
}

TEST(DashboardViewModelBrushTests, NetworkStatus_CaseSensitive_UnmatchedReturnsGray)
{
    auto result = DashboardBrushLogic::GetNetworkStatusColor(L"internet");
    EXPECT_EQ(TestColor::Gray, result);
}

// ===========================================
// CgnatStatusBrush Tests
// ===========================================

TEST(DashboardViewModelBrushTests, CgnatStatus_CGNAT_ReturnsRed)
{
    auto result = DashboardBrushLogic::GetCgnatStatusColor(L"CGNAT");
    EXPECT_EQ(TestColor::Red, result);
}

TEST(DashboardViewModelBrushTests, CgnatStatus_Open_ReturnsGreen)
{
    auto result = DashboardBrushLogic::GetCgnatStatusColor(L"Open");
    EXPECT_EQ(TestColor::Green, result);
}

TEST(DashboardViewModelBrushTests, CgnatStatus_Unknown_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetCgnatStatusColor(L"Unknown");
    EXPECT_EQ(TestColor::Gray, result);
}

TEST(DashboardViewModelBrushTests, CgnatStatus_Empty_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetCgnatStatusColor(L"");
    EXPECT_EQ(TestColor::Gray, result);
}

// ===========================================
// UpnpStatusBrush Tests
// ===========================================

TEST(DashboardViewModelBrushTests, UpnpStatus_Enabled_ReturnsGreen)
{
    auto result = DashboardBrushLogic::GetUpnpStatusColor(L"Enabled");
    EXPECT_EQ(TestColor::Green, result);
}

TEST(DashboardViewModelBrushTests, UpnpStatus_Disabled_ReturnsOrange)
{
    auto result = DashboardBrushLogic::GetUpnpStatusColor(L"Disabled");
    EXPECT_EQ(TestColor::Orange, result);
}

TEST(DashboardViewModelBrushTests, UpnpStatus_Unknown_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetUpnpStatusColor(L"Unknown");
    EXPECT_EQ(TestColor::Gray, result);
}

TEST(DashboardViewModelBrushTests, UpnpStatus_Empty_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetUpnpStatusColor(L"");
    EXPECT_EQ(TestColor::Gray, result);
}

// ===========================================
// NatTypeBrush Tests
// ===========================================

TEST(DashboardViewModelBrushTests, NatType_Open_ReturnsGreen)
{
    auto result = DashboardBrushLogic::GetNatTypeColor(L"Open");
    EXPECT_EQ(TestColor::Green, result);
}

TEST(DashboardViewModelBrushTests, NatType_Moderate_ReturnsOrange)
{
    auto result = DashboardBrushLogic::GetNatTypeColor(L"Moderate");
    EXPECT_EQ(TestColor::Orange, result);
}

TEST(DashboardViewModelBrushTests, NatType_Strict_ReturnsRed)
{
    auto result = DashboardBrushLogic::GetNatTypeColor(L"Strict");
    EXPECT_EQ(TestColor::Red, result);
}

TEST(DashboardViewModelBrushTests, NatType_Unknown_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetNatTypeColor(L"Unknown");
    EXPECT_EQ(TestColor::Gray, result);
}

TEST(DashboardViewModelBrushTests, NatType_Empty_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetNatTypeColor(L"");
    EXPECT_EQ(TestColor::Gray, result);
}

TEST(DashboardViewModelBrushTests, NatType_Symmetric_ReturnsGray)
{
    auto result = DashboardBrushLogic::GetNatTypeColor(L"Symmetric");
    EXPECT_EQ(TestColor::Gray, result);
}

// ===========================================
// StatusBrush Tests
// ===========================================

TEST(DashboardViewModelBrushTests, Status_HasError_ReturnsRed)
{
    auto result = DashboardBrushLogic::GetStatusColor(true);
    EXPECT_EQ(TestColor::Red, result);
}

TEST(DashboardViewModelBrushTests, Status_NoError_ReturnsLightGreen)
{
    auto result = DashboardBrushLogic::GetStatusColor(false);
    EXPECT_EQ(TestColor::LightGreen, result);
}

// ===========================================
// UpnpViewModel BoolToVisibility Tests
// ===========================================

enum class TestVisibility
{
    Visible,
    Collapsed
};

namespace UpnpViewModelLogic
{
    TestVisibility BoolToVisibility(bool value)
    {
        return value ? TestVisibility::Visible : TestVisibility::Collapsed;
    }
}

TEST(UpnpViewModelTests, BoolToVisibility_True_ReturnsVisible)
{
    auto result = UpnpViewModelLogic::BoolToVisibility(true);
    EXPECT_EQ(TestVisibility::Visible, result);
}

TEST(UpnpViewModelTests, BoolToVisibility_False_ReturnsCollapsed)
{
    auto result = UpnpViewModelLogic::BoolToVisibility(false);
    EXPECT_EQ(TestVisibility::Collapsed, result);
}
