#define BOOST_MP_STANDALONE
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>

using namespace boost::multiprecision;
using cpp_dec_float_50 = number<cpp_dec_float<50>>;

// Алгоритм Кэхана
long double kahanSum(const std::vector<long double>& nums) {
    long double sum = 0.0L;
    long double c = 0.0L;
    for (long double num : nums) {
        long double y = num - c;
        long double t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

long double KahanBabushkaKleinSum(const std::vector<long double>& input) {
    long double sum = 0.0L;
    long double cs  = 0.0L;
    long double ccs = 0.0L;

    for (size_t i = 0; i < input.size(); ++i) {
        long double c, cc;
        long double t = sum + input[i];
        if (std::fabs(sum) >= std::fabs(input[i])) {
            c = (sum - t) + input[i];
        } else {
            c = (input[i] - t) + sum;
        }
        sum = t;
        t = cs + c;
        if (std::fabs(cs) >= std::fabs(c)) {
            cc = (cs - t) + c;
        } else {
            cc = (c - t) + cs;
        }
        cs = t;
        ccs = ccs + cc;
    }
    return sum + (cs + ccs);
}

// Сумма с высокой точностью
cpp_dec_float_50 highPrecisionSum(const std::vector<long double>& nums) {
    cpp_dec_float_50 sum = 0;
    for (long double num : nums) {
        sum += cpp_dec_float_50(num);
    }
    return sum;
}

// Подсчёт количества совпадающих символов
int countMatchingDigits(const std::string& s1, const std::string& s2) {
    int count = 0;
    for (size_t i = 0; i < std::min(s1.size(), s2.size()); ++i) {
        if (s1[i] == s2[i])
            ++count;
        else
            break;
    }
    return count;
}

void runShuffledTests(const std::vector<long double>& numbers,
    const std::function<long double(const std::vector<long double>&)>& sumFunc,
    const cpp_dec_float_50& highPrecResult,
    const std::string& strHigh) {
    // Копируем вектор, чтобы оригинальный порядок не изменился
    std::vector<long double> numbersCopy = numbers;
    std::mt19937 rng(std::random_device{}());
    const int numRuns = 5; //numbers.size();

    for (int i = 1; i <= numRuns; ++i) {
        std::shuffle(numbersCopy.begin(), numbersCopy.end(), rng);
        long double shuffledSum = sumFunc(numbersCopy);
        cpp_dec_float_50 shuffledPrec(shuffledSum);
        cpp_dec_float_50 diff = abs(shuffledPrec - highPrecResult);

        std::ostringstream ossShuffled;
        ossShuffled << std::fixed << std::setprecision(50) << shuffledPrec;
        std::string strShuffled = ossShuffled.str();

        int matched = countMatchingDigits(strShuffled, strHigh);

        std::cout << "\nRun #" << i << " (shuffled):" << std::endl;
        std::cout << "Calculated sum:         " << strShuffled << std::endl;
        std::cout << "Difference from exact:  " << diff << std::endl;
        std::cout << "Matching digits:        " << matched << std::endl;
    }
    std::cout << "\n";
}

void tests(const std::vector<long double>& numbers) {

    cpp_dec_float_50 highPrecResult = highPrecisionSum(numbers);
    // Сумма по Кэхану
    long double kahanOriginal = kahanSum(numbers);
    cpp_dec_float_50 kahanPrecise(kahanOriginal);

    // Сумма по Кляину
    long double kahanBabushkaKleinResult = KahanBabushkaKleinSum(numbers);
    cpp_dec_float_50 kahanBabushkaKleinPrecise(kahanBabushkaKleinResult);

    // Абсолютная ошибка в высокой точности
    cpp_dec_float_50 error1 = abs(kahanPrecise - highPrecResult);

    cpp_dec_float_50 error2 = abs(kahanBabushkaKleinPrecise - highPrecResult);

    // Строки с высокой точностью
    std::ostringstream ossKahan, ossHigh, ossKlein;
    ossKahan << std::fixed << std::setprecision(50) << kahanPrecise;
    ossHigh << std::fixed << std::setprecision(50) << highPrecResult;
    ossKlein << std::fixed << std::setprecision(50) << kahanBabushkaKleinPrecise;

    std::string strKahan = ossKahan.str();
    std::string strHigh = ossHigh.str();
    std::string strKlein = ossKlein.str();
    int matchDigitsKahan = countMatchingDigits(strKahan, strHigh);
    int matchDigitsKlein = countMatchingDigits(strKlein, strHigh);

    std::cout << "Original Kahan sum:     " << strKahan << std::endl;
    std::cout << "High precision sum:     " << strHigh << std::endl;
    std::cout << "Absolute error:         " << error1 << std::endl;
    std::cout << "Matching digits:        " << matchDigitsKahan << std::endl;
    std::cout << "\n";

    std::cout << "Original Klein sum:     " << strKlein << std::endl;
    std::cout << "High precision sum:     " << strHigh << std::endl;
    std::cout << "Absolute error:         " << error2 << std::endl;
    std::cout << "Matching digits:        " << matchDigitsKlein << std::endl;

    // Перемешивания
    std::cout << "shuffle with Kahan Sum" << std::endl;
    runShuffledTests(numbers, kahanSum, highPrecResult, strHigh);
    std::cout << "shuffle with BabushkaKlein Sum" << std::endl;
    runShuffledTests(numbers, KahanBabushkaKleinSum, highPrecResult, strHigh);

}

void runTestToFile(const std::string& testName, const std::string& description, const std::vector<long double>& numbers, std::ofstream &outFile) {
    outFile << "\n=========================================\n";
    outFile << "Test: " << testName << "\n";
    outFile << "Описание: " << description << "\n";
    outFile << "Входные данные (" << numbers.size() << " элементов): ";
    outFile << "\nРезультаты теста:\n";

    // Перенаправляем std::cout в файл
    std::streambuf* oldBuf = std::cout.rdbuf();
    std::cout.rdbuf(outFile.rdbuf());

    // Вызов тестовой функции
    tests(numbers);

    // Восстанавливаем стандартный вывод
    std::cout.rdbuf(oldBuf);
    outFile << "\n=========================================\n";
}

// Функция, запускающая все тесты и записывающая результаты в файл
void runAllTestsToFile() {
    std::ofstream outFile("test_results.txt");
    if (!outFile) {
        std::cerr << "Ошибка открытия файла для записи!" << std::endl;
        return;
    }

    // Test 1: Потеря маленького числа при суммировании
    {
        std::vector<long double> numbers1 = {0.1234567890987654321L, -10000.0L};
        std::string desc = "Проверка, когда маленькое число теряется из-за большого.";
        runTestToFile("Test 1", desc, numbers1, outFile);
    }

    // Test 2: Числа, сильно различающиеся по порядку величины
    {
        std::vector<long double> numbers2 = {1000000.0L, 1e-8L};
        std::string desc = "Проверка суммирования чисел, отличающихся на несколько порядков (1e6 и 1e-8).";
        runTestToFile("Test 2", desc, numbers2, outFile);
    }

    // Test 3: Пример, когда Kahan может 'ломаться'
    {
        std::vector<long double> numbers3 = {1e20L, 1e-20L, -1e20L, 0.5L};
        std::string desc = "Проверка поглощения малого числа (1e-20) при суммировании с большими (1e20).";
        runTestToFile("Test 3", desc, numbers3, outFile);
    }

    // Test 4: Большие числа с противоположными знаками
    {
        std::vector<long double> numbers4 = {1e10L, -1e10L, 1e-5L, -1e-5L};
        std::string desc = "Проверка точности вычитания больших чисел с противоположными знаками.";
        runTestToFile("Test 4", desc, numbers4, outFile);
    }

    // Test 5: Повторяющееся маленькое значение
    {
        std::vector<long double> numbers5;
        for (long int i = 1; i <= 100000; ++i) {
            numbers5.push_back(0.1L);
        }
        std::string desc = "Проверка накопления ошибок при повторном суммировании 0.1 (100000 элементов).";
        runTestToFile("Test 5", desc, numbers5, outFile);
    }

    // Test 6: Маленькие значения с добавлением одного большого числа в конце
    {
        std::vector<long double> numbers6(100000, 1e-10L);
        numbers6.push_back(1.0L);
        std::string desc = "Проверка чувствительности порядка суммирования: множество маленьких чисел и один большой в конце.";
        runTestToFile("Test 6", desc, numbers6, outFile);
    }

    // Test 7: Большое количество маленьких значений + большой элемент в конце (перемешивание)
    {
        std::vector<long double> numbers7;
        for (int i = 0; i < 10000; ++i) {
            numbers7.push_back(1e-8L);
        }
        numbers7.push_back(1.0L); // большой в конце
        std::string desc = "Проверка влияния порядка: множество чисел 1e-8 с одним большим элементом (1.0) в конце.";
        runTestToFile("Test 7", desc, numbers7, outFile);
    }

    // Test 8: Суммирование вектора с обратным порядком (убывающая последовательность)
    {
        std::vector<long double> numbers8;
        for (int i = 1000000; i >= 1; --i) {
            numbers8.push_back(1.0L / i);
        }
        std::string desc = "Проверка влияния порядка суммирования при суммировании ряда 1/i (убывающая последовательность).";
        runTestToFile("Test 8", desc, numbers8, outFile);
    }

    // Test 9: Гармонический ряд (ряда 1/n²)
    {
        std::vector<long double> numbers9;
        for (int i = 1; i <= 10000; ++i) {
            numbers9.push_back(1.0L / (i * i));
        }
        std::string desc = "Проверка суммирования гармонического ряда 1/n² (ожидаемое значение ~π²/6).";
        runTestToFile("Test 9", desc, numbers9, outFile);
    }

    // Test 10: Компенсация машинного шума
    {
        std::vector<long double> numbers10 = {1e16L, 1.0L, -1e16L};
        std::string desc = "Проверка компенсации машинного шума, когда большое число 'поглощает' маленькое.";
        runTestToFile("Test 10", desc, numbers10, outFile);
    }

    outFile.close();
    std::cout << "Все тесты записаны в файл test_results.txt" << std::endl;
}

int main() {
    // Вызов функции, которая запускает все тесты и записывает результаты в файл
    runAllTestsToFile();
    return 0;
}


//{1e100, 1.0, -1e100, 3.14, 2.718, 1e-10, -1e-10}
