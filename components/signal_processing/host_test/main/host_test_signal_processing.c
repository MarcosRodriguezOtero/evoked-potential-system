#include <stdlib.h>

#include "unity.h"
#include "signal_processing.h"

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

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_encontrar_picos_detects_single_peak);

    return UNITY_END();
}
