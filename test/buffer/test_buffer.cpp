#include <Arduino.h>
#include <unity.h>
#include "buffer/buffer.h"

const char *TEST_STRING_NO_SPACES = "ThisStringDoesntContainEnySpaces";
const char *TEST_SENTENCE = "Reading Jojo is kind of like those pretend fights you had when you were younger.";
const char nulls[CommandBuffer::MAX_LENGTH] = {'\0'};

void test_constructing_buffer()
{
    // first of all buffer should be full of NULLs
    CommandBuffer b;
    TEST_ASSERT_EQUAL_CHAR_ARRAY(nulls, b.c_ptr(), CommandBuffer::MAX_LENGTH);
}

void test_adding_chars()
{
    CommandBuffer b;

    // make sure the entire string will fit in
    TEST_ASSERT_LESS_OR_EQUAL(CommandBuffer::MAX_LENGTH, strlen(TEST_STRING_NO_SPACES));

    // check if every char is added successfully
    for (size_t i = 0; i < strlen(TEST_STRING_NO_SPACES); i++)
        TEST_ASSERT_TRUE(b.push_back(TEST_STRING_NO_SPACES[i]));

    // entire string should have been coppied
    TEST_ASSERT_EQUAL_STRING(TEST_STRING_NO_SPACES, b.c_ptr());
}

void test_adding_string()
{
    CommandBuffer b;

    // make sure the entire string will fit in
    TEST_ASSERT_LESS_OR_EQUAL(CommandBuffer::MAX_LENGTH, strlen(TEST_STRING_NO_SPACES));

    // check if every char is added successfully
    TEST_ASSERT_TRUE(b.push_back(TEST_STRING_NO_SPACES));

    // entire string should have been coppied
    TEST_ASSERT_EQUAL_STRING(TEST_STRING_NO_SPACES, b.c_ptr());
}

void test_overflowing_buffer()
{
    CommandBuffer b;

    // adding 1 to make sure strings overflow the buffer
    size_t cannot_fit = CommandBuffer::MAX_LENGTH / strlen(TEST_SENTENCE) + 1;

    // check if strings are actually larger than buffer max length
    TEST_ASSERT_GREATER_THAN(CommandBuffer::MAX_LENGTH, strlen(TEST_SENTENCE) * cannot_fit);

    for (size_t i = 0; i < cannot_fit - 1; i++)
        TEST_ASSERT_TRUE(b.push_back(TEST_SENTENCE));

    // check twice
    // first time should go to the buffer boundery
    // second will start on it
    TEST_ASSERT_FALSE(b.push_back(TEST_SENTENCE));
    TEST_ASSERT_FALSE(b.push_back(TEST_SENTENCE));
}

void test_clearing()
{
    CommandBuffer b;

    // add string
    TEST_ASSERT_TRUE(b.push_back(TEST_SENTENCE));

    b.clear();
    // now buffer should be full of NULLs and equal to nulls table
    TEST_ASSERT_EQUAL_CHAR_ARRAY(nulls, b.c_ptr(), CommandBuffer::MAX_LENGTH);
}

void test_finding_words()
{
    CommandBuffer b;

    TEST_ASSERT_TRUE(b.push_back(TEST_SENTENCE));

    // copy string to a modifiable array
    char *test_string_one_copy = strdup(TEST_SENTENCE);

    char *to_tokenize = test_string_one_copy;
    size_t i = 0;
    const char *token = nullptr;
    // each of the words from parsed array should correspond to one word in text string
    while ((token = strtok_r(to_tokenize, " ", &to_tokenize)) != nullptr)
    {
        TEST_ASSERT_EQUAL_STRING(token, b.word_at(i));
        i++;
    }
    // make sure that stupid indexes don't work
    for (; i < 100; i++)
        TEST_ASSERT_NULL(b.word_at(i));
    // strdup allocates memory
    free(test_string_one_copy);
}

void test_positive_numbers()
{
    CommandBuffer b;
    char text_buffer[200U];
    int numbers[] = {
        10,
        20,
        100,
        312};
    // 1st number is ok
    // 2nd has non digit char 'a'
    // 3rd has 'o', but at the beggining
    // 4th one is ok
    sprintf(text_buffer, "%d  %da o%d     %d ", numbers[0], numbers[1], numbers[2], numbers[3]);
    TEST_ASSERT_LESS_OR_EQUAL(CommandBuffer::MAX_LENGTH, strlen(text_buffer));

    TEST_ASSERT_TRUE(b.push_back(text_buffer));

    TEST_ASSERT_TRUE(b.int_at(0).success);
    TEST_ASSERT_EQUAL_INT(numbers[0], b.int_at(0).value);

    TEST_ASSERT_FALSE(b.int_at(1).success);
    TEST_ASSERT_FALSE(b.int_at(2).success);

    TEST_ASSERT_TRUE(b.int_at(3).success);
    TEST_ASSERT_EQUAL_INT(numbers[3], b.int_at(3).value);

    TEST_ASSERT_FALSE(b.int_at(4).success);

    b.clear();

    // once again test if clearing the buffer works fine
    TEST_ASSERT_FALSE(b.int_at(0).success);
    TEST_ASSERT_NULL(b.command());
}

void test_long_positive_numbers()
{
    CommandBuffer b;
    // too big buffer just in case
    char text_buffer[200U] = {'\0'};
    int numbers[] = {2000, 3000, 4000, 10000, __INT_MAX__};
    // add all the numbers to buffer as text
    sprintf(text_buffer, "%d  %d %d %d    %d", numbers[0], numbers[1], numbers[2], numbers[3], numbers[4]);

    // make sure that buffer length doesn't exceed CommandBuffer capacity
    TEST_ASSERT_LESS_THAN(CommandBuffer::MAX_LENGTH, strlen(text_buffer));
    TEST_ASSERT_TRUE(b.push_back(text_buffer));

    for (size_t i = 0; i < sizeof(numbers) / sizeof(int); i++)
    {
        TEST_ASSERT_TRUE(b.int_at(i).value);
        TEST_ASSERT_EQUAL(numbers[i], b.int_at(i).value);
    }
}

void test_negative_numbers()
{
    // this test is literally the same as the test_positive_numbers
    // the only difference is that we will use negative numbers
    CommandBuffer b;
    char text_buffer[200U];
    int numbers[] = {
        -10,
        -20,
        -100,
        -312};

    sprintf(text_buffer, "%d  %da o%d     %d ", numbers[0], numbers[1], numbers[2], numbers[3]);

    // make sure that buffer length doesn't exceed CommandBuffer capacity
    TEST_ASSERT_LESS_OR_EQUAL(CommandBuffer::MAX_LENGTH, strlen(text_buffer));

    TEST_ASSERT_TRUE(b.push_back(text_buffer));

    TEST_ASSERT_TRUE(b.int_at(0).success);
    TEST_ASSERT_EQUAL_INT(numbers[0], b.int_at(0).value);

    TEST_ASSERT_FALSE(b.int_at(1).success);
    TEST_ASSERT_FALSE(b.int_at(2).success);

    TEST_ASSERT_TRUE(b.int_at(3).success);
    TEST_ASSERT_EQUAL_INT(numbers[3], b.int_at(3).value);

    TEST_ASSERT_FALSE(b.int_at(4).success);

    b.clear();

    // once again test if clearing the buffer works fine
    TEST_ASSERT_FALSE(b.int_at(0).success);
    TEST_ASSERT_NULL(b.command());
}

void test_floats()
{
    CommandBuffer b;
    char text_buffer[200U];
    float floats[] = {1.0f, 2.0f, 3.0f, 3.5f, 10.5f, 20.0f};
    sprintf(text_buffer, " %2.4f  %2.4f %2.4f %2.4f %2.4f    %2.4f 2.  3  2. .0 10..", floats[0], floats[1], floats[2], floats[3], floats[4], floats[5]);

    // make sure that buffer length doesn't exceed CommandBuffer capacity
    TEST_ASSERT_LESS_OR_EQUAL(CommandBuffer::MAX_LENGTH, strlen(text_buffer));

    TEST_ASSERT_TRUE(b.push_back(text_buffer));

    // check if all numbers are in the buffer
    for (size_t i = 0; i < sizeof(floats) / sizeof(float); i++)
    {
        TEST_ASSERT_TRUE(b.float_at(i).success);
        TEST_ASSERT_EQUAL_FLOAT(floats[i], b.float_at(i).value);
    }

    // and make sure that unwanted trash is not recognized as a float
    for (size_t i = sizeof(floats) / sizeof(float); i < 100; i++)
        TEST_ASSERT_FALSE(b.float_at(i).success);
}

void test_negative_floats()
{
    CommandBuffer b;
    char text_buffer[200U];
    float floats[] = {-1.0f, -2.0f, -3.0f, -3.5f, -10.5f, -20.0f};
    sprintf(text_buffer, " %2.4f  %2.4f %2.4f %2.4f %2.4f    %2.4f 2.  3  2. .0 10..", floats[0], floats[1], floats[2], floats[3], floats[4], floats[5]);

    // make sure that buffer length doesn't exceed CommandBuffer capacity
    TEST_ASSERT_LESS_OR_EQUAL(CommandBuffer::MAX_LENGTH, strlen(text_buffer));
    TEST_ASSERT_TRUE(b.push_back(text_buffer));

    // check if all numbers are in the buffer
    for (size_t i = 0; i < sizeof(floats) / sizeof(float); i++)
    {
        TEST_ASSERT_TRUE(b.float_at(i).success);
        TEST_ASSERT_EQUAL_FLOAT(floats[i], b.float_at(i).value);
    }

    // and make sure that unwanted trash is not recognized as a float
    for (size_t i = sizeof(floats) / sizeof(float); i < 100; i++)
        TEST_ASSERT_FALSE(b.float_at(i).success);
}

void test_positive_and_negative_floats_and_ints()
{
    CommandBuffer b;
    int ints[] = {21, -30, 1215, -3124, 0};
    float floats[] = {30.24f, -10251.12f};
    char text_buffer[200U];
    sprintf(text_buffer, "  %d  %d   %d %d  %d %2.4f  %7.2f   --40", ints[0], ints[1], ints[2], ints[3], ints[4], floats[0], floats[1]);

    TEST_ASSERT_LESS_OR_EQUAL(CommandBuffer::MAX_LENGTH, strlen(text_buffer));
    TEST_ASSERT_TRUE(b.push_back(text_buffer));

    // check if all int are found
    for (size_t i = 0; i < sizeof(ints) / sizeof(int); i++)
    {
        TEST_ASSERT_TRUE(b.int_at(i).success);
        TEST_ASSERT_EQUAL_INT(ints[i], b.int_at(i).value);
    }

    // same goes for floats
    for (size_t i = sizeof(ints) / sizeof(int); i < sizeof(ints) / sizeof(int) + sizeof(floats) / sizeof(float); i++)
    {
        TEST_ASSERT_TRUE(b.float_at(i).success);
        TEST_ASSERT_EQUAL_FLOAT(floats[i - sizeof(ints) / sizeof(int)], b.float_at(i).value);
    }
}

void test_real_life_command()
{
    CommandBuffer b;
    const char *command = "MP3_COMMAND 20 30 40";

    TEST_ASSERT_TRUE(b.push_back(command));

    TEST_ASSERT_EQUAL_CHAR_ARRAY(command, b.command(), strlen(b.command()));
    TEST_ASSERT_FALSE(b.int_at(0).success);
    TEST_ASSERT_EQUAL_INT(20, b.int_at(1).value);
    TEST_ASSERT_EQUAL_INT(30, b.int_at(2).value);
    TEST_ASSERT_EQUAL_INT(40, b.int_at(3).value);
    TEST_ASSERT_FALSE(b.int_at(4).success);

    b.clear();

    TEST_ASSERT_FALSE(b.int_at(0).success);
    TEST_ASSERT_NULL(b.command());
}

void setup()
{
    UNITY_BEGIN();
    RUN_TEST(test_constructing_buffer);
    RUN_TEST(test_adding_chars);
    RUN_TEST(test_adding_string);
    RUN_TEST(test_overflowing_buffer);
    RUN_TEST(test_clearing);
    RUN_TEST(test_finding_words);
    RUN_TEST(test_positive_numbers);
    RUN_TEST(test_long_positive_numbers);
    RUN_TEST(test_floats);
    RUN_TEST(test_negative_floats);
    RUN_TEST(test_positive_and_negative_floats_and_ints);
    RUN_TEST(test_real_life_command);
    UNITY_END();
}

void loop()
{
}