#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "unity.h"
#include "signal_processing.h"
#include "unity_config.h"


int cuentaP25 = 0;
int campo = 0;
float numeros1[2000];
float numeros2[2000];
double t[1000];

static const double DOUBLE_EPSILON = 1e-9;


enum {
    CHANGE_SIGN_SAMPLE_COUNT = 1000,
    CHANGE_SIGN_LONG_SAMPLE_COUNT = 1001
};


static void test_changeSign_null(void)
{
    changeSign(NULL);
}


static void test_changeSign_1000_samples(void)
{
    double signal[CHANGE_SIGN_SAMPLE_COUNT];
    for (int i = 0; i < CHANGE_SIGN_SAMPLE_COUNT; i++) {
        signal[i] = (double)(i + 1);
    }

    changeSign(signal);

    for (int i = 0; i < CHANGE_SIGN_SAMPLE_COUNT; i++) {
        TEST_ASSERT_EQUAL_DOUBLE(-(double)(i + 1), signal[i]);
    }
}


static void test_changeSign_long_buffer(void)
{
    double signal[CHANGE_SIGN_LONG_SAMPLE_COUNT];
    for (int i = 0; i < CHANGE_SIGN_LONG_SAMPLE_COUNT; i++) {
        signal[i] = (double)(i + 1);
    }

    changeSign(signal);

    for (int i = 0; i < CHANGE_SIGN_SAMPLE_COUNT; i++) {
        TEST_ASSERT_EQUAL_DOUBLE(-(double)(i + 1), signal[i]);
    }
    TEST_ASSERT_EQUAL_DOUBLE(CHANGE_SIGN_LONG_SAMPLE_COUNT, signal[CHANGE_SIGN_SAMPLE_COUNT]);
}


static void test_changeSign_twice(void)
{
    double signal[CHANGE_SIGN_SAMPLE_COUNT];
    for (int i = 0; i < CHANGE_SIGN_SAMPLE_COUNT; i++) {
        signal[i] = (double)(i + 1);
    }

    changeSign(signal);
    changeSign(signal);

    for (int i = 0; i < CHANGE_SIGN_SAMPLE_COUNT; i++) {
        TEST_ASSERT_EQUAL_DOUBLE((double)(i + 1), signal[i]);
    }
}


static void test_changeSign_signed_and_boundary_values(void)
{
    double signal[CHANGE_SIGN_SAMPLE_COUNT];
    for (int i = 0; i < CHANGE_SIGN_SAMPLE_COUNT; i++) {
        signal[i] = 0.0;
    }
    signal[0] = -2.5;
    signal[1] = 0.0;
    signal[2] = -0.0;
    signal[3] = -DBL_MAX;
    signal[4] = DBL_MAX;
    signal[5] = -DBL_MIN;
    signal[6] = DBL_MIN;

    changeSign(signal);

    TEST_ASSERT_EQUAL_DOUBLE(2.5, signal[0]);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, signal[1]);
    TEST_ASSERT_TRUE(signbit(signal[1]));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, signal[2]);
    TEST_ASSERT_FALSE(signbit(signal[2]));
    TEST_ASSERT_EQUAL_DOUBLE(DBL_MAX, signal[3]);
    TEST_ASSERT_EQUAL_DOUBLE(-DBL_MAX, signal[4]);
    TEST_ASSERT_EQUAL_DOUBLE(DBL_MIN, signal[5]);
    TEST_ASSERT_EQUAL_DOUBLE(-DBL_MIN, signal[6]);
}



static void test_encontrar_picos_detects_single_peak(void)
{
    double datos[]   = {0.0, 1.0, 0.0};
    double tiempos[] = {0.0, 1.0, 2.0};

    ResultadoPicos resultado = encontrar_picos(datos, tiempos, 3);

    TEST_ASSERT_EQUAL_INT(1, resultado.num_picos);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, resultado.picos[0].valor);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, resultado.picos[0].tiempo);

    free(resultado.picos);
}


static void test_encontrar_picos_finds_multiple_negative_and_positive_peaks(void)
{
    double datos[]   = {-5.0, -2.0, -4.0, -1.0, -6.0, 2.0, -3.0};
    double tiempos[] = {0.0, 1.5, 4.0, 8.0, 10.0, 11.0, 20.0};

    ResultadoPicos resultado = encontrar_picos(datos, tiempos, 7);

    TEST_ASSERT_EQUAL_INT(3, resultado.num_picos);
    TEST_ASSERT_EQUAL_DOUBLE(-2.0, resultado.picos[0].valor);
    TEST_ASSERT_EQUAL_DOUBLE(1.5, resultado.picos[0].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(-1.0, resultado.picos[1].valor);
    TEST_ASSERT_EQUAL_DOUBLE(8.0, resultado.picos[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, resultado.picos[2].valor);
    TEST_ASSERT_EQUAL_DOUBLE(11.0, resultado.picos[2].tiempo);

    free(resultado.picos);
}


static void test_encontrar_picos_detects_flat_peak_at_first_plateau_index(void)
{
    double datos[]   = {0.0, 4.0, 4.0, 1.0};
    double tiempos[] = {0.0, 2.5, 7.5, 12.0};

    ResultadoPicos resultado = encontrar_picos(datos, tiempos, 4);

    TEST_ASSERT_EQUAL_INT(1, resultado.num_picos);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, resultado.picos[0].valor);
    TEST_ASSERT_EQUAL_DOUBLE(2.5, resultado.picos[0].tiempo);

    free(resultado.picos);
}


static void test_encontrar_picos_does_not_detect_flat_region_without_peak(void)
{
    double datos[]   = {0.0, 2.0, 2.0, 3.0};
    double tiempos[] = {0.0, 1.0, 2.0, 3.0};

    ResultadoPicos resultado = encontrar_picos(datos, tiempos, 4);

    TEST_ASSERT_EQUAL_INT(0, resultado.num_picos);

    free(resultado.picos);
}


static void test_encontrar_picos_detects_long_flat_peak_at_first_plateau_index(void)
{
    double datos[]   = {0.0, 4.0, 4.0, 4.0, 1.0};
    double tiempos[] = {0.0, 2.5, 5.0, 7.5, 10.0};

    ResultadoPicos resultado = encontrar_picos(datos, tiempos, 5);

    TEST_ASSERT_EQUAL_INT(1, resultado.num_picos);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, resultado.picos[0].valor);
    TEST_ASSERT_EQUAL_DOUBLE(2.5, resultado.picos[0].tiempo);

    free(resultado.picos);
}


static void test_encontrar_picos_returns_none_for_monotonic_signal(void)
{
    double datos[]   = {0.0, 1.0, 2.0, 3.0};
    double tiempos[] = {0.0, 1.0, 2.0, 3.0};

    ResultadoPicos resultado = encontrar_picos(datos, tiempos, 4);

    TEST_ASSERT_EQUAL_INT(0, resultado.num_picos);
    free(resultado.picos);
}


static void test_encontrar_picos_ignores_endpoint_maxima(void)
{
    double datos[]   = {5.0, 2.0, 1.0, 2.0, 5.0};
    double tiempos[] = {0.0, 1.0, 2.0, 3.0, 4.0};

    ResultadoPicos resultado = encontrar_picos(datos, tiempos, 5);

    TEST_ASSERT_EQUAL_INT(0, resultado.num_picos);
    free(resultado.picos);
}


static void test_encontrar_picos_returns_none_for_two_samples(void)
{
    double datos[]   = {1.0, 2.0};
    double tiempos[] = {0.0, 1.0};

    ResultadoPicos resultado = encontrar_picos(datos, tiempos, 2);

    TEST_ASSERT_EQUAL_INT(0, resultado.num_picos);
    free(resultado.picos);
}


static void test_detectarP25_returns_null_without_in_window_peaks(void)
{
    double signal[] = {0.0, 1.0, 0.0};
    double times[]  = {10.0, 11.0, 12.0};
    cuentaP25 = 0;

    Pico *peaks = detectarP25(signal, times, 3);

    TEST_ASSERT_NULL(peaks);
    TEST_ASSERT_EQUAL_INT(0, cuentaP25);
}


static void test_detectarP25_returns_single_in_window_peak(void)
{
    double signal[] = {0.0, 4.0, 0.0};
    double times[]  = {0.0, 19.5, 40.0};
    cuentaP25 = 0;

    Pico *peaks = detectarP25(signal, times, 3);

    TEST_ASSERT_NOT_NULL(peaks);
    TEST_ASSERT_EQUAL_INT(1, cuentaP25);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, peaks[0].valor);
    TEST_ASSERT_EQUAL_DOUBLE(19.5, peaks[0].tiempo);
    free(peaks);
}


static void test_detectarP25_includes_window_bounds_and_sorts_by_amplitude(void)
{
    double signal[] = {0.0, 1.0, 0.0, 2.0, 0.0, 5.0, 0.0, 7.0, 0.0, 4.0, 0.0};
    double times[]  = {0.0, 19.49, 0.0, 19.5, 0.0, 32.0, 0.0, 32.01, 0.0, 21.0, 0.0};
    cuentaP25 = 0;

    Pico *peaks = detectarP25(signal, times, 11);

    TEST_ASSERT_NOT_NULL(peaks);
    TEST_ASSERT_EQUAL_INT(3, cuentaP25);
    TEST_ASSERT_EQUAL_DOUBLE(5.0, peaks[0].valor);
    TEST_ASSERT_EQUAL_DOUBLE(32.0, peaks[0].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, peaks[1].valor);
    TEST_ASSERT_EQUAL_DOUBLE(21.0, peaks[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, peaks[2].valor);
    TEST_ASSERT_EQUAL_DOUBLE(19.5, peaks[2].tiempo);
    free(peaks);
}


static void test_detectarP25_returns_three_largest_peaks(void)
{
    double signal[] = {0.0, 2.0, 0.0, 7.0, 0.0, 4.0, 0.0, 6.0, 0.0};
    double times[]  = {20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0};
    cuentaP25 = 0;

    Pico *peaks = detectarP25(signal, times, 9);

    TEST_ASSERT_NOT_NULL(peaks);
    TEST_ASSERT_EQUAL_INT(3, cuentaP25);
    TEST_ASSERT_EQUAL_DOUBLE(7.0, peaks[0].valor);
    TEST_ASSERT_EQUAL_DOUBLE(23.0, peaks[0].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(6.0, peaks[1].valor);
    TEST_ASSERT_EQUAL_DOUBLE(27.0, peaks[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(4.0, peaks[2].valor);
    TEST_ASSERT_EQUAL_DOUBLE(25.0, peaks[2].tiempo);
    free(peaks);
}


static void test_detectarP25_rejects_all_peaks_when_time_vector_is_zero(void)
{
    double signal[1000] = {0.0};
    double times[1000]  = {0.0};

    signal[200] = 1.0;
    signal[250] = 2.0;
    signal[300] = 3.0;

    cuentaP25 = 0;

    Pico *result = detectarP25(signal, times, 1000);

    TEST_ASSERT_NULL(result);
    TEST_ASSERT_EQUAL_INT(0, cuentaP25);
}


static void test_detectarP25_accumulates_cuentaP25_without_reset(void)
{
    double signal[] = {0.0, 4.0, 0.0};
    double times[]  = {0.0, 25.0, 40.0};
    cuentaP25 = 10;

    Pico *peaks = detectarP25(signal, times, 3);

    TEST_ASSERT_NOT_NULL(peaks);
    TEST_ASSERT_EQUAL_INT(11, cuentaP25);
    free(peaks);
}


static void test_detectarP25_increments_cuentaP25_by_selected_peaks(void)
{
    double signal[] = {0.0, 2.0, 0.0, 7.0, 0.0, 4.0, 0.0, 6.0, 0.0};
    double times[]  = {20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0};
    cuentaP25 = 5;

    Pico *peaks = detectarP25(signal, times, 9);

    TEST_ASSERT_NOT_NULL(peaks);
    TEST_ASSERT_EQUAL_INT(8, cuentaP25);
    free(peaks);
}


static void test_detectarN20_detects_peak_and_combines_amplitudes(void)
{
    double signal[1000] = {0.0};
    double times[1000];
    for (int i = 0; i < 1000; i++) {
        times[i] = (double)i / 10.0;
    }
    signal[200] = -0.4;
    Pico p25[] = {{0.2, 21.0}};
    cuentaP25 = 1;

    Pico *result = detectarN20(signal, times, 1000, p25);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(21.0, result[0].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(20.0, result[1].tiempo);
    TEST_ASSERT_DOUBLE_WITHIN(DOUBLE_EPSILON, 0.6, result[1].valor);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, result[2].valor);
    TEST_ASSERT_EQUAL_DOUBLE(0.4, signal[200]);
    free(result);
}


static void test_detectarN20_rejects_amplitude_below_threshold(void)
{
    double signal[1000] = {0.0};
    double times[1000];
    for (int i = 0; i < 1000; i++) {
        times[i] = (double)i / 10.0;
    }
    signal[200] = -0.39;
    Pico p25[] = {{0.2, 21.0}};
    cuentaP25 = 1;

    Pico *result = detectarN20(signal, times, 1000, p25);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(20.0, result[1].tiempo);
    TEST_ASSERT_TRUE(result[1].valor < 0.6);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result[2].valor);
    free(result);
}


static void test_detectarN20_accepts_lower_latency_boundary(void)
{
    double signal[1000] = {0.0};
    double times[1000];
    for (int i = 0; i < 1000; i++) {
        times[i] = (double)i / 10.0;
    }
    Pico p25[] = {{0.2, 17.0}};
    cuentaP25 = 1;
    signal[160] = -0.4;

    Pico *result = detectarN20(signal, times, 1000, p25);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(16.0, result[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, result[2].valor);
    free(result);
}


static void test_detectarN20_accepts_upper_latency_boundary(void)
{
    double signal[1000] = {0.0};
    double times[1000];
    for (int i = 0; i < 1000; i++) {
        times[i] = (double)i / 10.0;
    }
    signal[230] = -0.4;
    Pico p25[] = {{0.2, 24.0}};
    cuentaP25 = 1;

    Pico *result = detectarN20(signal, times, 1000, p25);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(23.0, result[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, result[2].valor);
    free(result);
}


static void test_detectarN20_rejects_below_lower_latency_boundary(void)
{
    double signal[1000] = {0.0};
    double times[1000];
    for (int i = 0; i < 1000; i++) {
        times[i] = (double)i / 10.0;
    }
    Pico p25[] = {{0.2, 17.0}};
    cuentaP25 = 1;
    signal[159] = -0.4;

    Pico *result = detectarN20(signal, times, 1000, p25);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(15.9, result[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result[2].valor);
    free(result);
}


static void test_detectarN20_rejects_above_upper_latency_boundary(void)
{
    double signal[1000] = {0.0};
    double times[1000];
    for (int i = 0; i < 1000; i++) {
        times[i] = (double)i / 10.0;
    }
    signal[231] = -0.4;
    Pico p25[] = {{0.2, 24.0}};
    cuentaP25 = 1;

    Pico *result = detectarN20(signal, times, 1000, p25);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(23.1, result[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, result[2].valor);
    free(result);
}


static void test_detectarN20_selects_preceding_peak_and_nearest_P25(void)
{
    double signal[1000] = {0.0};
    double times[1000];
    for (int i = 0; i < 1000; i++) {
        times[i] = (double)i / 10.0;
    }
    signal[180] = -1.0;
    signal[200] = -2.0;
    Pico p25[] = {
        {1.0, 24.0},
        {1.0, 22.0}
    };
    cuentaP25 = 2;

    Pico *result = detectarN20(signal, times, 1000, p25);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(20.0, result[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(22.0, result[0].tiempo);
    TEST_ASSERT_DOUBLE_WITHIN(DOUBLE_EPSILON, 3.0, result[1].valor);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, result[2].valor);
    free(result);
}


static void test_detectarN20_requires_P25_more_than_0_5_after_N20(void)
{
    double signal[1000] = {0.0};
    double times[1000];

    for (int i = 0; i < 1000; i++) {
        times[i] = (double)i / 10.0;
    }

    signal[200] = -1.0;
    Pico p25[] = {
        {5.0, 20.5},
        {1.0, 20.6}
    };
    cuentaP25 = 2;

    Pico *result = detectarN20(signal, times, 1000, p25);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_DOUBLE(20.0, result[1].tiempo);
    TEST_ASSERT_EQUAL_DOUBLE(20.6, result[0].tiempo);
    TEST_ASSERT_DOUBLE_WITHIN(DOUBLE_EPSILON, 2.0, result[1].valor);
    TEST_ASSERT_EQUAL_DOUBLE(1.0, result[2].valor);
    free(result);
}


static void test_algoritmo_returns_2_no_peaks_when_first_P25_is_missing(void)
{
    double *signal1 = (double *)numeros1;
    double *signal2 = (double *)numeros2;
    cuentaP25 = 0;
    campo = 0;
    for (int i = 0; i < 1000; i++) {
        signal1[i] = 0.0;
        signal2[i] = 0.0;
        t[i] = (double)i / 10.0;
    }

    int result = algoritmo();

    TEST_ASSERT_EQUAL_INT(2, result);
}


static void test_algoritmo_returns_5_N20_detected_when_both_channels_detect_N20(void)
{
    double *signal1 = (double *)numeros1;
    double *signal2 = (double *)numeros2;
    cuentaP25 = 0;
    campo = 0;
    for (int i = 0; i < 1000; i++) {
        signal1[i] = 0.0;
        signal2[i] = 0.0;
        t[i] = (double)i / 10.0;
    }
    signal1[200] = -0.4;
    signal1[210] = 0.2;
    signal2[200] = -0.4;
    signal2[210] = 0.2;

    int result = algoritmo();

    TEST_ASSERT_EQUAL_INT(5, result);
}


static void test_algoritmo_returns_4_inconclusive_when_channels_disagree(void)
{
    double *signal1 = (double *)numeros1;
    double *signal2 = (double *)numeros2;
    cuentaP25 = 0;
    campo = 0;
    for (int i = 0; i < 1000; i++) {
        signal1[i] = 0.0;
        signal2[i] = 0.0;
        t[i] = (double)i / 10.0;
    }
    signal1[200] = -0.4;
    signal1[210] = 0.2;
    signal2[200] = -0.39;
    signal2[210] = 0.2;

    int result = algoritmo();

    TEST_ASSERT_EQUAL_INT(4, result);
}


static void test_algoritmo_returns_6_N20_not_detected_when_both_channels_reject_N20(void)
{
    double *signal1 = (double *)numeros1;
    double *signal2 = (double *)numeros2;
    cuentaP25 = 0;
    campo = 0;
    for (int i = 0; i < 1000; i++) {
        signal1[i] = 0.0;
        signal2[i] = 0.0;
        t[i] = (double)i / 10.0;
    }
    signal1[200] = -0.39;
    signal1[210] = 0.2;
    signal2[200] = -0.39;
    signal2[210] = 0.2;

    int result = algoritmo();

    TEST_ASSERT_EQUAL_INT(6, result);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    // changeSign tests
    RUN_TEST(test_changeSign_null);
    RUN_TEST(test_changeSign_1000_samples);
    RUN_TEST(test_changeSign_long_buffer);
    RUN_TEST(test_changeSign_twice);
    RUN_TEST(test_changeSign_signed_and_boundary_values);

    // encontrar_picos tests
    RUN_TEST(test_encontrar_picos_detects_single_peak);
    RUN_TEST(test_encontrar_picos_finds_multiple_negative_and_positive_peaks);
    RUN_TEST(test_encontrar_picos_detects_flat_peak_at_first_plateau_index);
    RUN_TEST(test_encontrar_picos_does_not_detect_flat_region_without_peak);
    RUN_TEST(test_encontrar_picos_detects_long_flat_peak_at_first_plateau_index);
    RUN_TEST(test_encontrar_picos_returns_none_for_monotonic_signal);
    RUN_TEST(test_encontrar_picos_ignores_endpoint_maxima);
    RUN_TEST(test_encontrar_picos_returns_none_for_two_samples);

    // detectarP25 tests
    RUN_TEST(test_detectarP25_returns_null_without_in_window_peaks);
    RUN_TEST(test_detectarP25_returns_single_in_window_peak);
    RUN_TEST(test_detectarP25_includes_window_bounds_and_sorts_by_amplitude);
    RUN_TEST(test_detectarP25_returns_three_largest_peaks);
    RUN_TEST(test_detectarP25_rejects_all_peaks_when_time_vector_is_zero);
    RUN_TEST(test_detectarP25_accumulates_cuentaP25_without_reset);
    RUN_TEST(test_detectarP25_increments_cuentaP25_by_selected_peaks);

    // detectarN20 tests
    RUN_TEST(test_detectarN20_detects_peak_and_combines_amplitudes);
    RUN_TEST(test_detectarN20_rejects_amplitude_below_threshold);
    RUN_TEST(test_detectarN20_accepts_lower_latency_boundary);
    RUN_TEST(test_detectarN20_accepts_upper_latency_boundary);
    RUN_TEST(test_detectarN20_rejects_below_lower_latency_boundary);
    RUN_TEST(test_detectarN20_rejects_above_upper_latency_boundary);
    RUN_TEST(test_detectarN20_selects_preceding_peak_and_nearest_P25);
    RUN_TEST(test_detectarN20_requires_P25_more_than_0_5_after_N20);

    // algoritmo tests
    RUN_TEST(test_algoritmo_returns_2_no_peaks_when_first_P25_is_missing);
    RUN_TEST(test_algoritmo_returns_5_N20_detected_when_both_channels_detect_N20);
    RUN_TEST(test_algoritmo_returns_4_inconclusive_when_channels_disagree);
    RUN_TEST(test_algoritmo_returns_6_N20_not_detected_when_both_channels_reject_N20);

    return UNITY_END();
}
