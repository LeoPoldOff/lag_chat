#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <pthread.h> 
#include <locale.h>


int input_esc;

int getche() 
{
	int ch;
	struct termios oldt, newt;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON );
//	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}
//
// getch() - ввод одного символа без эхо
//
int getch() {
	int ch;
	struct termios oldt, newt;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
//	newt.c_lflag &= ~( ICANON );
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

void SetCursorPos(int XPos, int YPos)
{
    printf("\033[%d;%dH", YPos+1, XPos+1);
}

void SetInputCursor()
{
    SetCursorPos(5, 25);
    printf(">");
}

void GetAuthScreen()
{
    char auth_screen[4096];
    FILE *fp;
    srand(time(NULL));
    char *file_name[32];
    int val = rand();

    for (int i = 0; i < 5; i++)
        val = rand()%2;
    printf("%d\n", val);
    if (val == 1)
    {
        *file_name = "auth_02.txt";
    }
    else
    {
        *file_name = "auth_01.txt";
    }
    fp = fopen(*file_name, "r");

    SetCursorPos(0, 0);
    while (fgets(auth_screen, 2812, fp) != NULL)
        printf("%s", auth_screen);

    fclose(fp);
    SetCursorPos(29, 14);
}

int getNumberPosition(int counter[], int symbol_counter)
{
    int result = 0;
    for (int i = 0; i < symbol_counter; i++)
        result = result + counter[i];
    return result;
}

void VerticalCursorMove(int left_shift, int symbol_count, int shift_y)
{
    int shift_x = left_shift + symbol_count;
    SetCursorPos(shift_x, shift_y);
}

void HorizontalCursorMove(int direction, int left_shift,  int shift_y, int max_symbol_count, int* current_symbol)
{
    if (direction < 0)
    {
        if (*current_symbol > 0)
            *current_symbol = *current_symbol - 1;
        int shift_x = left_shift + *current_symbol;
        SetCursorPos(shift_x, shift_y);
    }
    else
    {
        if (*current_symbol < max_symbol_count)
            *current_symbol = *current_symbol + 1;
        int shift_x = left_shift + *current_symbol;
        SetCursorPos(shift_x, shift_y);
    }      
}

void PrintNewSequence(int left_shift,  int shift_y, int current_symbol, int max_symbol_count, char input[], int symbols_counter[])
{
    SetCursorPos(left_shift , shift_y);
    int empty_cell_count = 0;
    for (int i = 0; i < max_symbol_count; i++)
    {
        int pos_pointer = getNumberPosition(symbols_counter, i) + empty_cell_count;
        int count_symbols = symbols_counter[i];
        if (count_symbols == 0)
        {
            empty_cell_count++;
            printf(" ");
        }
        if (count_symbols == 1)
            printf("%c", input[pos_pointer]);
        if (count_symbols == 2)
            printf("%c%c", input[pos_pointer], input[pos_pointer + 1]);
    }
    
    // Обновление переменных
    SetCursorPos(left_shift + current_symbol, shift_y);
}

void ShiftSequence(int shift_size, int current_element_number, int last_element_number, char input[], int symbols_counter[])
{
    int new_last_pointer = getNumberPosition(symbols_counter, last_element_number) + shift_size;
    int end_coping = getNumberPosition(symbols_counter, current_element_number + 1);
    for (int i = new_last_pointer; i >= end_coping; i--)
    {
        input[i] = input[i - shift_size];
    }
    
    for (int i = last_element_number + 1; i > current_element_number; i--)
        symbols_counter[i] = symbols_counter[i - 1];
}

void AnalizeInputChar(int code, char array[], int is_pass, int* current_symbol, int* max_symbol_count, int symbols_counter[], int left_shift, int shift_y)
{
    int position = getNumberPosition(symbols_counter, *current_symbol);

    int insert = 0;
    int increment = 1;
    if (code > 31 && code < 127)
    {
        if (symbols_counter[*current_symbol] == 0)
        {         
            symbols_counter[*current_symbol] = 1;
            
            array[position] = (char)code;

            *current_symbol = *current_symbol + 1;
            *max_symbol_count = *max_symbol_count + 1;

            if (!is_pass)
                printf("%c", code);
            else
                printf("*");
        }
        else
        {
            symbols_counter[*current_symbol] = 1;

            ShiftSequence(1, *current_symbol, *max_symbol_count, array, symbols_counter);
            insert = 1;
            
            array[position] = (char)code;

            *current_symbol = *current_symbol + 1;
            *max_symbol_count = *max_symbol_count + 1;
        }
    }
    if (code == 208 || code == 209)
    {
        char value = getch();
        switch (symbols_counter[*current_symbol])
        {
            case 0:
            {
                symbols_counter[*current_symbol] = 2;
            
                array[position] = (char)code;
                array[position + 1] = (char)value;

                *current_symbol = *current_symbol + 1;
                *max_symbol_count = *max_symbol_count + 1;

                if (!is_pass)
                    printf("%c%c", code, value);
                else
                    printf("*");
                break;
            }
            default:
            {
                symbols_counter[*current_symbol] = 2;

                ShiftSequence(2, *current_symbol, *max_symbol_count, array, symbols_counter);
                insert = 1;
                
                array[position] = (char)code;
                array[position + 1] = (char)value;

                *current_symbol = *current_symbol + 1;
                *max_symbol_count = *max_symbol_count + 1;
                                
                break;
            }
        }   
    }

    if (insert)
        PrintNewSequence(left_shift, shift_y, *current_symbol, *max_symbol_count, array, symbols_counter);

    // printf("\nPO-RUSSIAN:\n");
    // for (int i = 0; array[i]; i++)
    //     printf("input[%d] = %d\n", i, array[i]);
    // printf("\n");

    // printf("CURRENT: %d\n", *current_symbol);
    // printf("MAX: %d\n", *max_symbol_count);
    // printf("\n");

    // for (int i = 0; array[i]; i++)
    //     printf("size[%d] = %d\n", i, symbols_counter[i]);
    // printf("\n");
}

void Zerolizer(char input[])
{
    for (int i = 0; i < 32; i++)
        input[i] = '\n';
}

void ZerolizerI(int input[])
{
    for (int i = 0; i < 32; i++)
        input[i] = 0;
}

void cleaner(int size_array[], char input[], int last_element)
{
    int start = getNumberPosition(size_array, last_element);
    for (int i = start; i < 64; i++)
        input[i] = '\n';
}

void BackspaceHandler(char input[], int left_shift, int shift_y, int* max_symbol_count, int* current_symbol, int symbols_counter[])
{
    if (*current_symbol == 0)
        return;

    // Логическая перезапись символов с того, что перед курсором, до последнего введённого 
    int pointer_position = *current_symbol - 1;
    int shift = symbols_counter[pointer_position];
    // printf("shift = %d\n", shift);

    char *buffer = malloc(64);
    int buffer_pointers[32];
    ZerolizerI(buffer_pointers);

    int end_shift = symbols_counter[*max_symbol_count];
    if (end_shift == 0)
        end_shift++;

    int start = getNumberPosition(symbols_counter, *current_symbol);
    int end = getNumberPosition(symbols_counter, *max_symbol_count) + end_shift;

    // printf("Start:%d\nEnd:%d\n", start, end);
    // printf("\n");

    for (int i = start; i < end; i++)
    {
        buffer[i - start] = input[i];
    }
    for (int i = 0; i < shift - 1; i++)
        buffer[i - start + end] = '\n';
    // for (int i = 0; i < end - start; i++)
    //     printf("buffer[%d]=%d\n", i, buffer[i]);
    int prev = getNumberPosition(symbols_counter, pointer_position);
    for (int i = 0; i < end - start + shift - 1; i++)
    {
        input[prev + i] = buffer[i];   
    }

    for (int i = *current_symbol; i <= *max_symbol_count; i++)
    {
        buffer_pointers[i - *current_symbol] = symbols_counter[i];
    }
    // for (int i = 0; i < 4; i++)
    //    printf("buffer_pointers[%d]=%d\n", i, buffer_pointers[i]);
    // printf("OK!\n");
    for (int i = 0; i <= *max_symbol_count - *current_symbol; i++)
    {
        symbols_counter[i + *current_symbol - 1] = buffer_pointers[i];
    }
    // for (int i = 0; i < 4; i++)
    //    printf("symbols_counter[%d]=%d\n", i, symbols_counter[i]);
    // printf("OK!\n");    

    // Печать в консоль того, как сделали все свои дела
    *max_symbol_count = *max_symbol_count - 1;
    *current_symbol = *current_symbol - 1;

    PrintNewSequence(left_shift, shift_y, *current_symbol, *max_symbol_count + 1, input, symbols_counter);    
}


int AuthHandler()
{
    

    int input_char;
    int flag = 1;

    int is_login = 1;

    int login_symbols_pointer = 0;   
    int login_counter[32] = {0};
    char *login = malloc(64);
    Zerolizer(login);


    int password_symbols_pointer = 0;
    int password_counter[32] = {0};
    char *password = malloc(64);
    Zerolizer(password);

    int password_max_symbols_counter = 0;
    int login_max_symbols_counter = 0;
    
    int left_shift = 29;
    int login_position = 14;
    int password_position = 18;

    system("clear");
    GetAuthScreen();
    while (flag)
    {
        input_char = getch();
        switch (input_char)
        {
            case 10:
                {
                    if (is_login)
                    {
                        is_login = 0;
                        SetCursorPos(left_shift, password_position);
                    }
                    else
                    {
                        system("clear");
                        printf("Press Enter\n");
                        printf(login, "\n");
                        printf(password, "\n");
                        flag = 0;
                    }
                    break;
                }
            case 27:
                {
                    input_char = getch();
                    if (input_char == 91)
                    {
                        input_char = getch();
                        switch (input_char)
                        {
                            case 65:
                            {
                                if (is_login)
                                {
                                    is_login = 0;
                                    VerticalCursorMove(left_shift, password_symbols_pointer, password_position);
                                }
                                else
                                {
                                    is_login = 1;
                                    VerticalCursorMove(left_shift, login_symbols_pointer, login_position);
                                }
                                break;
                            }
                            case 66:
                            {
                                if (is_login)
                                {
                                    is_login = 0;
                                    VerticalCursorMove(left_shift, password_symbols_pointer, password_position);
                                }
                                else
                                {
                                    is_login = 1;
                                    VerticalCursorMove(left_shift, login_symbols_pointer, login_position);
                                }
                                break;
                            }
                            case 67:
                            {
                                if (is_login)
                                    HorizontalCursorMove(1, left_shift, login_position, login_max_symbols_counter, &login_symbols_pointer);
                                else
                                    HorizontalCursorMove(1, left_shift, password_position, password_max_symbols_counter, &password_symbols_pointer);  
                                break;
                            }
                            case 68:
                            {
                                if (is_login)
                                    HorizontalCursorMove(-1, left_shift, login_position, login_max_symbols_counter, &login_symbols_pointer);
                                else
                                    HorizontalCursorMove(-1, left_shift, login_position, login_max_symbols_counter, &login_symbols_pointer); 
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    else
                    {
                        system("clear");
                        printf("Exit\n");
                        exit(0);
                    }
                    break;
                }        
            
            case 127: 
            {
                if (is_login)
                {
                    if (login_symbols_pointer > 0)
                        BackspaceHandler(login, left_shift, login_position,
                         &login_max_symbols_counter, &login_symbols_pointer, login_counter);
                }
                else
                {
                    if (password_symbols_pointer > 0)
                        BackspaceHandler(password, left_shift, password_position, 
                        &password_max_symbols_counter, &password_symbols_pointer, password_counter);
                }
                break;
            }
            default:
            {
                if (is_login)
                {
                    if (login_symbols_pointer < 32)
                    {

                        // SetCursorPos(0, 35);
                        
                        // for (int i = 0; login[i]; i++)
                        //     printf("%d ", login[i]);
                        // printf("\n");
                        // for (int i = 0; login_counter[i]; i++)
                        //     printf("%d ", login_counter[i]);
                        // printf("\n");
                        // printf("\n\nmax: %d\ncurrent: %d\n", login_max_symbols_counter, login_symbols_pointer);

                        // SetCursorPos(left_shift + login_symbols_pointer, login_position);

                        AnalizeInputChar(input_char, login, 0, &login_symbols_pointer, &login_max_symbols_counter, login_counter, left_shift, login_position);
                        
                        // SetCursorPos(0, 44);
                        
                        // for (int i = 0; login[i]; i++)
                        //     printf("%d ", login[i]);
                        // printf("\n");
                        // for (int i = 0; login_counter[i]; i++)
                        //     printf("%d ", login_counter[i]);
                        // printf("\n");
                        // printf("\n\nmax: %d\ncurrent: %d\n", login_max_symbols_counter, login_symbols_pointer);

                        // SetCursorPos(left_shift + login_symbols_pointer, login_position);
                    }
                }
                else
                {
                    if (password_symbols_pointer < 32)
                    {
                        AnalizeInputChar(input_char, password, 1, &password_symbols_pointer, &password_max_symbols_counter, password_counter, left_shift, password_position);
                        SetCursorPos(left_shift + password_symbols_pointer, password_position);
                    }
                }
                
                break;
            }
        }
    }
   return 0;
}


// BackspaceHandler(char input[], int left_shift, int shift_y, int* max_symbol_count, int* current_symbol, int symbols_counter[])
// AnalizeInputChar(int code, char array[], int is_pass, int* current_symbol, int* max_symbol_count, int symbols_counter[])
int main()
{
    setlocale( LC_ALL, "");
    AuthHandler();


    // system("clear");

    // char im[] = "абвгдаеёжзиклмнопрстуфхцшщъыьэюяАБВГД";
    // for (int i = 0; im[i]; i++)
    //     printf("%d", im[i]);
    // printf("\n");

    // char input[] = {'b', 208, 176, '\n', '\n', '\n', '\n', '\n'};
    // int size[] = {1, 2, 0, 0, 0, 0, 0, 0};
    // int current = 2;
    // int max = 2;
    // int left = 0;
    // int y = 10;


    // PrintNewSequence(left, y, current, max, input, size);
    // printf("\n");

    // printf("\n");
    // for (int i = 0; input[i]; i++)
    //     printf("input[%d] = %d\n", i, input[i]);
    // printf("\n");
    // for (int i = 0; size[i]; i++)
    //     printf("size[%d] = %d\n", i, size[i]);
    // printf("\n");

    // int code = getch();
    // AnalizeInputChar(code, input, 0, &current, &max, size, left, y);

    // for (int i = 0; size[i]; i++)
    //     printf("size[%d] = %d\n", i, size[i]);
    // printf("\n");
    // for (int i = 0; input[i]; i++)
    //     printf("input[%d] = %d\n", i, input[i]);
    // printf("\n");
}