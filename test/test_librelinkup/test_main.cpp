#include <Arduino.h>
#include <unity.h>
#include <time.h>

#include "librelinkup.h"

namespace {

LIBRELINKUP librelinkup;

void reset_runtime_state() {
    librelinkup.login_data().email = "";
    librelinkup.login_data().password = "";
    librelinkup.login_data().user_id = "";
    librelinkup.login_data().account_id = "";
    librelinkup.login_data().user_country = "";
    librelinkup.login_data().user_token = "";
    librelinkup.login_data().connection_country = "";
    librelinkup.login_data().connection_status = 0;
    librelinkup.login_data().user_token_expires = 0;
    librelinkup.login_data().user_login_status = 0;

    memset(librelinkup.sensor_history_data().graph_data, 0, sizeof(librelinkup.sensor_history_data().graph_data));
    memset(librelinkup.sensor_history_data().timestamp, 0, sizeof(librelinkup.sensor_history_data().timestamp));
    memset(librelinkup.sensor_history_data().factory_timestamp, 0, sizeof(librelinkup.sensor_history_data().factory_timestamp));

    librelinkup.glucose_data().glucoseMeasurement = 0;
    librelinkup.glucose_data().trendArrow = 0;
    librelinkup.glucose_data().measurement_color = 0;
    librelinkup.glucose_data().str_TrendMessage = "";
    librelinkup.glucose_data().str_measurement_timestamp = "";
    librelinkup.glucose_data().str_trendArrow = "";
    librelinkup.glucose_data().str_measurement_factorytimestamp = "";
    librelinkup.glucose_data().glucosetargetLow = 0;
    librelinkup.glucose_data().glucosetargetHigh = 0;
    librelinkup.glucose_data().glucoseAlarmLow = 0;
    librelinkup.glucose_data().glucoseAlarmHigh = 0;
    librelinkup.glucose_data().glucosefixedLowAlarmValues = 0;

    librelinkup.sensor_data().sensor_state = 0;
    librelinkup.sensor_data().sensor_sn_non_active = "";
    librelinkup.sensor_data().sensor_id_non_active = "";
    librelinkup.sensor_data().sensor_non_activ_unixtime = 0;
    librelinkup.sensor_data().sensor_id = "";
    librelinkup.sensor_data().sensor_sn = "";
    librelinkup.sensor_data().sensor_runtime = 0;
    librelinkup.sensor_data().sensor_activation_time = 0;

    librelinkup.sensor_lifetime().sensor_valid_days = 0;
    librelinkup.sensor_lifetime().sensor_valid_hours = 0;
    librelinkup.sensor_lifetime().sensor_valid_minutes = 0;
    librelinkup.sensor_lifetime().sensor_valid_seconds = 0;

    librelinkup.status().timestamp_status = 0;
    librelinkup.status().sensor_state = 0;
    librelinkup.status().last_timestamp_unixtime = 0;
    librelinkup.timezone_offset() = 1;
    librelinkup.reconnect_flag() = false;
    librelinkup.expired_flag() = false;
    librelinkup.tz_locked = false;
    librelinkup.tz_offset_h_locked = 0;
    librelinkup.tz_offset_s_locked = 0;
}

void test_extract_host_strips_scheme_path_and_port() {
    TEST_ASSERT_EQUAL_STRING(
        "api-de.libreview.io",
        LIBRELINKUP::extractHost(" https://api-de.libreview.io:443/llu/connections ").c_str());
    TEST_ASSERT_EQUAL_STRING(
        "example.com",
        LIBRELINKUP::extractHost("example.com/path").c_str());
}

void test_region_to_base_url_maps_eu_and_fallback() {
    TEST_ASSERT_EQUAL_STRING(
        "https://api-de.libreview.io",
        LIBRELINKUP::regionToBaseUrl("de").c_str());
    TEST_ASSERT_EQUAL_STRING(
        "https://api-de.libreview.io",
        LIBRELINKUP::regionToBaseUrl("eu").c_str());
    TEST_ASSERT_EQUAL_STRING(
        "https://api.libreview.io",
        LIBRELINKUP::regionToBaseUrl("us").c_str());
}

void test_credentials_and_sensitive_runtime_cleanup() {
    reset_runtime_state();

    librelinkup.set_credentials("user@example.com", "secret");
    librelinkup.login_data().user_token = "abcdef1234567890";
    librelinkup.login_data().user_token_expires = 123456789;

    TEST_ASSERT_TRUE(librelinkup.has_credentials());
    TEST_ASSERT_TRUE(librelinkup.password_set());
    TEST_ASSERT_TRUE(librelinkup.token_present());
    TEST_ASSERT_EQUAL_STRING("abcdef...7890", librelinkup.masked_user_token().c_str());

    librelinkup.clear_sensitive_runtime_data();

    TEST_ASSERT_FALSE(librelinkup.has_credentials());
    TEST_ASSERT_FALSE(librelinkup.password_set());
    TEST_ASSERT_FALSE(librelinkup.token_present());
    TEST_ASSERT_EQUAL_UINT32(0, librelinkup.login_data().user_token_expires);
}

void test_masked_user_token_handles_short_tokens() {
    reset_runtime_state();
    librelinkup.login_data().user_token = "short";

    TEST_ASSERT_EQUAL_STRING("***", librelinkup.masked_user_token(2, 2).c_str());
}

void test_sha256_matches_known_reference() {
    TEST_ASSERT_EQUAL_STRING(
        "fcdec6df4d44dbc637c7c5b58efface52a7f8a88535423430255be0bb89bedd8",
        librelinkup.account_id_sha256("user-123").c_str());
}

void test_check_sensor_type_orders_serials_lexicographically() {
    TEST_ASSERT_EQUAL(1, librelinkup.check_sensor_type("0J999999", "0J000000"));
    TEST_ASSERT_EQUAL(-1, librelinkup.check_sensor_type("0A999999", "0J000000"));
    TEST_ASSERT_EQUAL(0, librelinkup.check_sensor_type("0J000000", "0J000000"));
}

void test_parse_timestamp_handles_am_pm_edges() {
    setenv("TZ", "UTC0", 1);
    tzset();

    TEST_ASSERT_EQUAL_INT64(
        1768775140,
        librelinkup.parseTimestamp("1/18/2026 10:25:40 PM"));
    TEST_ASSERT_EQUAL_INT64(
        1768694400,
        librelinkup.parseTimestamp("1/18/2026 12:00:00 AM"));
    TEST_ASSERT_EQUAL_INT64(
        1768737600,
        librelinkup.parseTimestamp("1/18/2026 12:00:00 PM"));
}

void test_update_tz_offset_once_locks_one_hour_offset() {
    reset_runtime_state();
    setenv("TZ", "UTC0", 1);
    tzset();

    bool ok = librelinkup.update_tz_offset_once(
        "1/18/2026 10:25:40 PM",
        "1/18/2026 9:25:40 PM");

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_TRUE(librelinkup.tz_locked);
    TEST_ASSERT_EQUAL(1, librelinkup.tz_offset_h_locked);
    TEST_ASSERT_EQUAL_INT32(3600, librelinkup.tz_offset_s_locked);
}

void test_update_tz_offset_once_rejects_invalid_timestamp() {
    reset_runtime_state();

    bool ok = librelinkup.update_tz_offset_once(
        "invalid",
        "1/18/2026 9:25:40 PM");

    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_FALSE(librelinkup.tz_locked);
}

void test_ingest_graph_json_populates_measurement_and_history() {
    reset_runtime_state();
    setenv("TZ", "UTC0", 1);
    tzset();

    static const char payload[] = R"json({
      "data": {
        "connection": {
          "country": "DE",
          "status": 2,
          "targetLow": 70,
          "targetHigh": 180,
          "patientDevice": {
            "ll": 55,
            "hl": 240,
            "fixedLowAlarmValues": { "mgdl": 55 }
          },
          "glucoseMeasurement": {
            "ValueInMgPerDl": 123,
            "TrendArrow": 4,
            "MeasurementColor": 1,
            "TrendMessage": "Rising",
            "Timestamp": "1/18/2026 10:25:40 PM",
            "FactoryTimestamp": "1/18/2026 9:25:40 PM"
          },
          "sensor": {
            "sn": "0J123456",
            "deviceId": "inactive-sensor",
            "a": 1768600000
          }
        },
        "activeSensors": [
          {
            "sensor": {
              "deviceId": "active-sensor",
              "sn": "0J654321",
              "pt": 3,
              "a": 1768603600
            }
          }
        ],
        "graphData": [
          {
            "ValueInMgPerDl": 111,
            "Timestamp": "1/18/2026 10:20:40 PM",
            "FactoryTimestamp": "1/18/2026 9:20:40 PM"
          },
          {
            "ValueInMgPerDl": 115,
            "Timestamp": "1/18/2026 10:15:40 PM",
            "FactoryTimestamp": "1/18/2026 9:15:40 PM"
          }
        ]
      }
    })json";

    bool ok = librelinkup.ingest_graph_json(
        reinterpret_cast<const uint8_t*>(payload),
        strlen(payload));

    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_UINT16(123, librelinkup.glucose_data().glucoseMeasurement);
    TEST_ASSERT_EQUAL_UINT8(4, librelinkup.glucose_data().trendArrow);
    TEST_ASSERT_EQUAL_STRING("\xE2\x86\x97", librelinkup.glucose_data().str_trendArrow.c_str());
    TEST_ASSERT_EQUAL_STRING("Rising", librelinkup.glucose_data().str_TrendMessage.c_str());
    TEST_ASSERT_EQUAL_UINT16(70, librelinkup.glucose_data().glucosetargetLow);
    TEST_ASSERT_EQUAL_UINT16(180, librelinkup.glucose_data().glucosetargetHigh);
    TEST_ASSERT_EQUAL_STRING("DE", librelinkup.login_data().connection_country.c_str());
    TEST_ASSERT_EQUAL_INT16(2, librelinkup.login_data().connection_status);
    TEST_ASSERT_EQUAL_STRING("active-sensor", librelinkup.sensor_data().sensor_id.c_str());
    TEST_ASSERT_EQUAL_STRING("0J654321", librelinkup.sensor_data().sensor_sn.c_str());
    TEST_ASSERT_EQUAL_UINT16(111, librelinkup.sensor_history_data().graph_data[0]);
    TEST_ASSERT_EQUAL_UINT16(115, librelinkup.sensor_history_data().graph_data[1]);
    TEST_ASSERT_EQUAL_UINT16(123, librelinkup.sensor_history_data().graph_data[141]);
    TEST_ASSERT_EQUAL_INT64(1768774840, librelinkup.sensor_history_data().timestamp[0]);
    TEST_ASSERT_EQUAL_INT64(1768771240, librelinkup.sensor_history_data().factory_timestamp[0]);
    TEST_ASSERT_TRUE(librelinkup.tz_locked);
    TEST_ASSERT_EQUAL_UINT8(2, librelinkup.check_graphdata());
    TEST_ASSERT_TRUE(librelinkup.get_last_graph_json().length() > 0);
}

void test_ingest_graph_json_rejects_invalid_json() {
    reset_runtime_state();

    static const char payload[] = "{not-json";
    bool ok = librelinkup.ingest_graph_json(
        reinterpret_cast<const uint8_t*>(payload),
        strlen(payload));

    TEST_ASSERT_FALSE(ok);
}

}  // namespace

void setup() {
    delay(1000);
    UNITY_BEGIN();

    RUN_TEST(test_extract_host_strips_scheme_path_and_port);
    RUN_TEST(test_region_to_base_url_maps_eu_and_fallback);
    RUN_TEST(test_credentials_and_sensitive_runtime_cleanup);
    RUN_TEST(test_masked_user_token_handles_short_tokens);
    RUN_TEST(test_sha256_matches_known_reference);
    RUN_TEST(test_check_sensor_type_orders_serials_lexicographically);
    RUN_TEST(test_parse_timestamp_handles_am_pm_edges);
    RUN_TEST(test_update_tz_offset_once_locks_one_hour_offset);
    RUN_TEST(test_update_tz_offset_once_rejects_invalid_timestamp);
    RUN_TEST(test_ingest_graph_json_populates_measurement_and_history);
    RUN_TEST(test_ingest_graph_json_rejects_invalid_json);

    UNITY_END();
}

void loop() {}
