#include <catch_amalgamated.hpp>

#include "../../../src/mve/math/math.hpp"

TEST_CASE("Test Math Functions", "[Math Functions]")
{
    SECTION("is_equal_approx")
    {
        REQUIRE(mve::is_equal_approx(1.0f, 2.0f) == false);
        REQUIRE(mve::is_equal_approx(1.0f, 1.0f) == true);
        REQUIRE(mve::is_equal_approx(1.0f, 1.000009f) == true);
        REQUIRE(mve::is_equal_approx(1.0f, 1.00001f) == false);
    }

    SECTION("is_zero_approx")
    {
        REQUIRE(mve::is_zero_approx(1.0f) == false);
        REQUIRE(mve::is_zero_approx(0.0f) == true);
        REQUIRE(mve::is_zero_approx(0.00001f) == false);
        REQUIRE(mve::is_zero_approx(0.000009f) == true);
    }

    SECTION("abs")
    {
        REQUIRE(mve::abs(1.0f) == 1.0f);
        REQUIRE(mve::abs(-1.0f) == 1.0f);
        REQUIRE(mve::abs(0.0f) == 0.0f);

        REQUIRE(mve::abs(1) == 1);
        REQUIRE(mve::abs(-1) == 1);
        REQUIRE(mve::abs(0) == 0);
    }

    SECTION("ceil")
    {
        REQUIRE(mve::ceil(1.0f) == 1.0f);
        REQUIRE(mve::ceil(1.1f) == 2.0f);
        REQUIRE(mve::ceil(1.5f) == 2.0f);
        REQUIRE(mve::ceil(1.9f) == 2.0f);

        REQUIRE(mve::ceil(-1.0f) == -1.0f);
        REQUIRE(mve::ceil(-1.1f) == -1.0f);
        REQUIRE(mve::ceil(-1.5f) == -1.0f);
        REQUIRE(mve::ceil(-1.9f) == -1.0f);
    }

    SECTION("clamp")
    {
        REQUIRE(mve::clamp(1.0f, -1.0f, 2.0f) == 1.0f);
        REQUIRE(mve::clamp(4.0f, -1.0f, 2.0f) == 2.0f);
        REQUIRE(mve::clamp(-10.0f, -1.0f, 2.0f) == -1.0f);

        REQUIRE(mve::clamp(1, -1, 2) == 1);
        REQUIRE(mve::clamp(4, -1, 2) == 2);
        REQUIRE(mve::clamp(-10, -1, 2) == -1);
    }

    SECTION("sqrt")
    {
        REQUIRE(mve::sqrt(4.0f) == 2.0f);
        REQUIRE(mve::sqrt(9.0f) == 3.0f);
        REQUIRE(mve::sqrt(1.0f) == 1.0f);
    }

    SECTION("pow")
    {
        REQUIRE(mve::pow(1.0f, 3.0f) == 1.0f);
        REQUIRE(mve::pow(2.0f, 3.0f) == 8.0f);
        REQUIRE(mve::is_equal_approx(mve::pow(2.0f, 1.5f), 2.82842712f));
        REQUIRE(mve::pow(2.0f, -3.0f) == 0.125f);
    }

    SECTION("sqrd")
    {
        REQUIRE(mve::sqrd(1.0f) == 1.0f);
        REQUIRE(mve::sqrd(2.0f) == 4.0f);
        REQUIRE(mve::sqrd(3.0f) == 9.0f);

        REQUIRE(mve::sqrd(-1.0f) == 1.0f);
        REQUIRE(mve::sqrd(-2.0f) == 4.0f);
        REQUIRE(mve::sqrd(-3.0f) == 9.0f);

        REQUIRE(mve::sqrd(1) == 1);
        REQUIRE(mve::sqrd(2) == 4);
        REQUIRE(mve::sqrd(3) == 9);

        REQUIRE(mve::sqrd(-1) == 1);
        REQUIRE(mve::sqrd(-2) == 4);
        REQUIRE(mve::sqrd(-3) == 9);
    }

    SECTION("floor")
    {
        REQUIRE(mve::floor(1.0f) == 1.0f);
        REQUIRE(mve::floor(1.1f) == 1.0f);
        REQUIRE(mve::floor(1.5f) == 1.0f);
        REQUIRE(mve::floor(1.9f) == 1.0f);

        REQUIRE(mve::floor(-1.0f) == -1.0f);
        REQUIRE(mve::floor(-1.1f) == -2.0f);
        REQUIRE(mve::floor(-1.5f) == -2.0f);
        REQUIRE(mve::floor(-1.9f) == -2.0f);
    }

    SECTION("sin")
    {
        REQUIRE(mve::sin(0.0f) == 0.0f);
        REQUIRE(mve::is_equal_approx(mve::sin(1.0f), 0.8414709848f));
        REQUIRE(mve::is_equal_approx(mve::sin(mve::pi), 0.0f));
        REQUIRE(mve::is_equal_approx(mve::sin(0.5f * mve::pi), 1.0f));
    }

    SECTION("asin")
    {
        REQUIRE(mve::asin(0.0f) == 0.0f);
        REQUIRE(mve::is_equal_approx(mve::asin(0.8414709848f), 1.0f));
        REQUIRE(mve::is_equal_approx(mve::asin(1.0f), 0.5f * mve::pi));
    }

    SECTION("cos")
    {
        REQUIRE(mve::cos(0.0f) == 1.0f);
        REQUIRE(mve::is_equal_approx(mve::cos(1.0f), 0.5403023059f));
        REQUIRE(mve::is_equal_approx(mve::cos(mve::pi), -1.0f));
        REQUIRE(mve::is_equal_approx(mve::cos(0.5f * mve::pi), 0.0f));
    }

    SECTION("acos")
    {
        REQUIRE(mve::acos(1.0f) == 0.0f);
        REQUIRE(mve::is_equal_approx(mve::acos(0.5403023059f), 1.0f));
        REQUIRE(mve::is_equal_approx(mve::acos(-1.0f), mve::pi));
        REQUIRE(mve::is_equal_approx(mve::acos(0.0f), 0.5f * mve::pi));
    }

    SECTION("tan")
    {
        REQUIRE(mve::tan(0.0f) == 0.0f);
        REQUIRE(mve::is_equal_approx(mve::tan(1.0f), 1.5574077247f));
        REQUIRE(mve::is_equal_approx(mve::tan(mve::pi), 0.0f));
        REQUIRE(mve::is_equal_approx(mve::tan(0.2f * mve::pi), 0.726542528f));
    }

    SECTION("atan")
    {
        REQUIRE(mve::atan(0.0f) == 0.0f);
        REQUIRE(mve::is_equal_approx(mve::atan(1.5574077247f), 1.0f));
        REQUIRE(mve::is_equal_approx(mve::atan(0.726542528f), 0.2f * mve::pi));
    }

    SECTION("atan2")
    {
        REQUIRE(mve::atan2(0.0f, 3.0f) == 0.0f);
        REQUIRE(mve::is_equal_approx(mve::atan2(3.0f, 4.0f), 0.6435011088f));
        REQUIRE(mve::is_equal_approx(mve::atan2(8.0f, 2.0f), 1.3258176637f));
    }

    SECTION("round")
    {
        REQUIRE(mve::round(1.0f) == 1.0f);
        REQUIRE(mve::round(1.1f) == 1.0f);
        REQUIRE(mve::round(1.5f) == 2.0f);
        REQUIRE(mve::round(1.9f) == 2.0f);

        REQUIRE(mve::round(-1.0f) == -1.0f);
        REQUIRE(mve::round(-1.1f) == -1.0f);
        REQUIRE(mve::round(-1.5f) == -2.0f);
        REQUIRE(mve::round(-1.9f) == -2.0f);
    }

    SECTION("radians")
    {
        REQUIRE(mve::radians(0.0f) == 0.0f);
        REQUIRE(mve::is_equal_approx(mve::radians(180.0f), mve::pi));
        REQUIRE(mve::is_equal_approx(mve::radians(360.0f), 2.0f * mve::pi));
        REQUIRE(mve::is_equal_approx(mve::radians(37.0f), 0.6457718f));
        REQUIRE(mve::is_equal_approx(mve::radians(-37.0f), -0.6457718f));
    }

    SECTION("degrees")
    {
        REQUIRE(mve::degrees(0.0f) == 0.0f);
        REQUIRE(mve::is_equal_approx(mve::degrees(mve::pi), 180.0f));
        REQUIRE(mve::is_equal_approx(mve::degrees(2.0f * mve::pi), 360.0f));
        REQUIRE(mve::is_equal_approx(mve::degrees(0.6457718f), 37.0f));
        REQUIRE(mve::is_equal_approx(mve::degrees(-0.6457718f), -37.0f));
    }

    SECTION("linear_interpolate")
    {
        REQUIRE(mve::linear_interpolate(0.0f, 2.0f, 0.0f) == 0.0f);
        REQUIRE(mve::linear_interpolate(0.0f, 2.0f, 1.0f) == 2.0f);
        REQUIRE(mve::linear_interpolate(0.0f, 2.0f, 0.5f) == 1.0f);
        REQUIRE(mve::linear_interpolate(-10.0f, 10.0f, 0.25f) == -5.0f);
    }

    SECTION("min")
    {
        REQUIRE(mve::min(1.0f, 1.0f) == 1.0f);
        REQUIRE(mve::min(-1.0f, 1.0f) == -1.0f);
        REQUIRE(mve::min(1.0f, -1.0f) == -1.0f);
        REQUIRE(mve::min(-100.0f, -1.0f) == -100.0f);
        REQUIRE(mve::min(100.0f, 1.0f) == 1.0f);
    }

    SECTION("max")
    {
        REQUIRE(mve::max(1.0f, 1.0f) == 1.0f);
        REQUIRE(mve::max(-1.0f, 1.0f) == 1.0f);
        REQUIRE(mve::max(1.0f, -1.0f) == 1.0f);
        REQUIRE(mve::max(-100.0f, -1.0f) == -1.0f);
        REQUIRE(mve::max(100.0f, 1.0f) == 100.0f);
    }

    SECTION("log2")
    {
        REQUIRE(mve::is_equal_approx(mve::log2(4.0f), 2.0f));
        REQUIRE(mve::is_equal_approx(mve::log2(8.0f), 3.0f));
        REQUIRE(mve::is_equal_approx(mve::log2(0.125f), -3.0f));
    }
}