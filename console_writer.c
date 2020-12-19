#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <pthread.h> 
#include <locale.h>
#include <semaphore.h>
#include "parser.c"

#define CHAT_BUFFER_L 150 * 16
#define USERS_POINTERS_L 45 * 16

enum MainBackground
{
    BackgoundInput = 0,
    BackgroundChat = 1,
    BackgroundUsers = 2
};


int SCREEN = BackgoundInput;

char INPUT[256] = {'\0'};
int INPUT_SIZER[256] = {0};
int INPUT_SIZER_POINTER = 0;
int INPUT_SIZER_LAST_SYMBOL_POINTER = 0;

char CURSOR_X = 0;
char CURSOR_Y = 0;

pthread_mutex_t CURSOR_MUTEX;

char CHAT[65536] = {'\0'};
int CHAT_POINTER = 0;

char USERS[4096] = {'\0'};

int SOCKET_HUY_EGO_ZNAET = 0;

int SHIFT_X = 39;
int SHIFT_Y = 28;


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
/*
void SetCursorPos(int XPos, int YPos)
{
    printf("\033[%d;%dH", YPos+1, XPos+1);
}
*/
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
        *file_name = "static/auth_02.txt";
    }
    else
    {
        *file_name = "static/auth_01.txt";
    }
    fp = fopen(*file_name, "r");

    SetCursorPos(0, 0);
    while (fgets(auth_screen, 2812, fp) != NULL)
        printf("%s", auth_screen);

    fclose(fp);
    SetCursorPos(82, 26);
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

void PrintNewSequence(int is_pass, int left_shift,  int shift_y, int current_symbol, int max_symbol_count, char input[], int symbols_counter[])
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
        {
            if (is_pass)
                printf("*");
            else
                printf("%c", input[pos_pointer]);    
        }
        if (count_symbols == 2)
        {
            if (is_pass)
                printf("*");
            else
                printf("%c%c", input[pos_pointer], input[pos_pointer + 1]);
        }
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
        PrintNewSequence(is_pass, left_shift, shift_y, *current_symbol, *max_symbol_count, array, symbols_counter);

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

void cleaner(int size_array[], char input[], int last_element)
{
    int start = getNumberPosition(size_array, last_element);
    for (int i = start; i < 64; i++)
        input[i] = '\n';
}

void BackspaceHandler(int is_pass, char input[], int left_shift, int shift_y, int* max_symbol_count, int* current_symbol, int symbols_counter[])
{
    if (*current_symbol == 0)
        return;

    // Логическая перезапись символов с того, что перед курсором, до последнего введённого 
    int pointer_position = *current_symbol - 1;
    int shift = symbols_counter[pointer_position];
    // printf("shift = %d\n", shift);

    char *buffer = malloc(64);
    int buffer_pointers[32] = {0};

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

    PrintNewSequence(is_pass, left_shift, shift_y, *current_symbol, *max_symbol_count + 1, input, symbols_counter);    
}

void AuthHandler(char log[], char pass[])
{  
    int input_char;
    int flag = 1;

    char login[64] = {'\n'};
    char password[64] = {'\n'};

    int is_login = 1;

    int login_symbols_pointer = 0;   
    int login_counter[32] = {0};


    int password_symbols_pointer = 0;
    int password_counter[32] = {0};

    int password_max_symbols_counter = 0;
    int login_max_symbols_counter = 0;
    
    int left_shift = 82;
    int login_position = 26;
    int password_position = 30;

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
                        BackspaceHandler(0, login, left_shift, login_position,
                         &login_max_symbols_counter, &login_symbols_pointer, login_counter);
                }
                else
                {
                    if (password_symbols_pointer > 0)
                        BackspaceHandler(1, password, left_shift, password_position, 
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
   
   for (int i = 0; i < 64; i++)
   {
       log[i] = login[i];
       pass[i] = password[i];
   }

}

// ============================================================================================================================================================
// ============================================================================================================================================================

void GetMainScreen()
{
    char main_screen[4096];
    FILE *fp;
    char *file_name[32];
    switch (SCREEN)
    {
        case BackgoundInput:
        {
            *file_name = "static/main_input.txt";
            break;
        }
        case BackgroundChat:
        {
            *file_name = "static/main_chat.txt";
            break;
        }
        case BackgroundUsers:
        {
            *file_name = "static/main_users.txt";
        }
        default:
        {
            system("clear");
            printf("Неизвестный бэкграунд\n");
            break;
        }
    }
    fp = fopen(*file_name, "r");
    SetCursorPos(0, 0);
    while (fgets(main_screen, 2812, fp) != NULL)
        printf("%s", main_screen);
    fclose(fp);
}

void CopyToBuffer(char source[], char target[], int start, int size)
{
    int last = start + size;
    for (int i = start; i < last; i++)
        target[i - start] = source[i];
}

void PrintContentChat()
{
    char content[16][256];
    char string_content[CHAT_BUFFER_L] = {'\0'};
    CopyToBuffer(CHAT, string_content, CHAT_POINTER, CHAT_BUFFER_L);
    int count = long_str_parser(content, string_content, 150);
    print_massive_in_x_y(string_content, count, 39, 9);
}

int getPointer(int symbol_number)
{
    int result = 0;
    for (int i = 0; i < symbol_number; i++)
        result = result + INPUT_SIZER[i];
    return result;
}

void PrintContentUsers()
{
    char content[16][256];
    char string_content[USERS_POINTERS_L] = {'\0'};
    int user_pointer = getPointer(INPUT_SIZER_POINTER);
    CopyToBuffer(USERS, string_content, user_pointer, USERS_POINTERS_L);
    int count = long_str_parser(content, string_content, 45);
    print_massive_in_x_y(string_content, count, 133, 9);
}

void PrintContentInput()
{
    SetCursorPos(SHIFT_X, SHIFT_Y);
    printf(INPUT);
}

void Update()
{
    pthread_mutex_lock(&CURSOR_MUTEX);
    GetMainScreen();
    PrintContentChat();
    PrintContentUsers();
    PrintContentInput();
    SetCursorPos(CURSOR_X, CURSOR_Y);
    pthread_mutex_unlock(&CURSOR_MUTEX);
}

int PressEnter(char* input, int sock_fd){
	char args[50][100];
	int res = request_parser(input, args);

    pthread_mutex_lock(&CURSOR_MUTEX);
    CURSOR_X = SHIFT_X;
    for (int i = 0; i < 256; i++)
        INPUT_SIZER[i] = 0;
    INPUT_SIZER_POINTER = 0;
    INPUT_SIZER_LAST_SYMBOL_POINTER = 0;

	if (res == -1){
		memset(input, '\0', 256);
        pthread_mutex_unlock(&CURSOR_MUTEX);
		return -1;
	}
	else if (res == 0 || res == 1){
		extern sendbuf(sock_fd, input);
		memset(input, '\0', 256);
        pthread_mutex_unlock(&CURSOR_MUTEX);
		return 1;
	}

}

void AnalizeChar(int code)
{
    if (INPUT_SIZER_LAST_SYMBOL_POINTER >= 256)
        return; 
    pthread_mutex_lock(&CURSOR_MUTEX);
    int position = getPointer(INPUT_SIZER_POINTER);
    int insert = 0;
    int increment = 1;
    if (code > 31 && code < 127)
    {
        if (INPUT_SIZER[INPUT_SIZER_POINTER] == 0)
        {         
            INPUT_SIZER[INPUT_SIZER_POINTER] = 1;
            
            INPUT[position] = (char)code;

            INPUT_SIZER_POINTER = INPUT_SIZER_POINTER + 1;
            INPUT_SIZER_LAST_SYMBOL_POINTER = INPUT_SIZER_LAST_SYMBOL_POINTER + 1;
        }
        else
        {
            INPUT_SIZER[INPUT_SIZER_POINTER] = 1;

            ShiftSequence(1, INPUT_SIZER_POINTER, INPUT_SIZER_LAST_SYMBOL_POINTER, INPUT, INPUT_SIZER);
            insert = 1;
            
            INPUT[position] = (char)code;

            INPUT_SIZER_POINTER = INPUT_SIZER_POINTER + 1;
            INPUT_SIZER_LAST_SYMBOL_POINTER = INPUT_SIZER_LAST_SYMBOL_POINTER + 1;
        }
    }
    if (code == 208 || code == 209)
    {
        char value = getch();
        switch (INPUT_SIZER[INPUT_SIZER_POINTER])
        {
            case 0:
            {
                INPUT_SIZER[INPUT_SIZER_POINTER] = 2;
            
                INPUT[position] = (char)code;
                INPUT[position + 1] = (char)value;

                INPUT_SIZER_POINTER = INPUT_SIZER_POINTER + 1;
                INPUT_SIZER_LAST_SYMBOL_POINTER = INPUT_SIZER_LAST_SYMBOL_POINTER + 1;
                break;
            }
            default:
            {
                INPUT_SIZER[INPUT_SIZER_POINTER] = 2;

                ShiftSequence(2, INPUT_SIZER_POINTER, INPUT_SIZER_LAST_SYMBOL_POINTER, INPUT, INPUT_SIZER);
                insert = 1;
                
                INPUT[position] = (char)code;
                INPUT[position + 1] = (char)value;

                INPUT_SIZER_POINTER = INPUT_SIZER_POINTER + 1;
                INPUT_SIZER_LAST_SYMBOL_POINTER = INPUT_SIZER_LAST_SYMBOL_POINTER + 1;
                                
                break;
            }
        }   
    }

    CURSOR_X = SHIFT_X + INPUT_SIZER_POINTER;
    pthread_mutex_unlock(&CURSOR_MUTEX);
}

void MoveCursorHorizontal(int direction)
{
    if (direction > 0)
    {
        if (INPUT_SIZER_POINTER < INPUT_SIZER_LAST_SYMBOL_POINTER)
            INPUT_SIZER_POINTER = INPUT_SIZER_POINTER + 1;
    }
    if (direction < 0)
    {
        if (INPUT_SIZER_POINTER > 0)
            INPUT_SIZER_POINTER = INPUT_SIZER_POINTER - 1;
    }
    CURSOR_X = SHIFT_X + INPUT_SIZER_POINTER;
}

void BackspaceAnalizer()
{
    if (INPUT_SIZER_POINTER == 0)
        return;
    pthread_mutex_lock(&CURSOR_MUTEX);
    
    int pointer_position = INPUT_SIZER_POINTER - 1;
    int shift = INPUT_SIZER[pointer_position];

    char *buffer = malloc(64);
    int buffer_pointers[32] = {0};

    int end_shift = INPUT_SIZER[INPUT_SIZER_LAST_SYMBOL_POINTER];
    if (end_shift == 0)
        end_shift++;

    int start = getPointer(INPUT_SIZER_POINTER);
    int end = getPointer(INPUT_SIZER_LAST_SYMBOL_POINTER) + end_shift;

    for (int i = start; i < end; i++)
    {
        buffer[i - start] = INPUT[i];
    }
    for (int i = 0; i < shift - 1; i++)
        buffer[i - start + end] = '\n';
    int prev = getPointer(pointer_position);
    for (int i = 0; i < end - start + shift - 1; i++)
    {
        INPUT[prev + i] = buffer[i];   
    }

    for (int i = INPUT_SIZER_POINTER; i <= INPUT_SIZER_LAST_SYMBOL_POINTER; i++)
    {
        buffer_pointers[i - INPUT_SIZER_POINTER] = INPUT_SIZER[i];
    }
    for (int i = 0; i <= INPUT_SIZER_LAST_SYMBOL_POINTER - INPUT_SIZER_POINTER; i++)
    {
        INPUT_SIZER[i + INPUT_SIZER_POINTER - 1] = buffer_pointers[i];
    }
    // Печать в консоль того, как сделали все свои дела
    INPUT_SIZER_LAST_SYMBOL_POINTER = INPUT_SIZER_LAST_SYMBOL_POINTER - 1;
    INPUT_SIZER_POINTER = INPUT_SIZER_POINTER - 1;
    
    CURSOR_X = SHIFT_X + INPUT_SIZER_POINTER;
    pthread_mutex_unlock(&CURSOR_MUTEX);
}

void MainHandler(char login[], char password[])
{
    int input_char;
    int flag = 1;
    Update();
    while (flag)
    {
        input_char = getch();
        switch (input_char)
        {
            // Enter
            case 10:
                {
                    PressEnter(INPUT, SOCKET_HUY_EGO_ZNAET);
                    break;
                }
            // ESCAPE-SEQUENCE
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
                                // Change current window
                                break;
                            }
                            case 66:
                            {
                                // Change current window
                                break;
                            }
                            case 67:
                            {
                                // Move right
                                MoveCursorHorizontal(1);
                                break;
                            }
                            case 68:
                            {
                                // Move left
                                MoveCursorHorizontal(-1); 
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    else
                    {
                        // Exit
                        flag = 0;
                    }
                    break;
                }        
            // Backspace
            case 127: 
            {
                BackspaceAnalizer();
                break;
            }
            default:
            {
                // Some input
                AnalizeChar(input_char);
                break;
            }
        }
        Update();
    }
}


// BackspaceHandler(char input[], int left_shift, int shift_y, int* max_symbol_count, int* current_symbol, int symbols_counter[])
// AnalizeInputChar(int code, char array[], int is_pass, int* current_symbol, int* max_symbol_count, int symbols_counter[])
int main()
{
    setlocale( LC_ALL, "");
    pthread_mutex_init(&CURSOR_MUTEX, NULL);

    CURSOR_X = 39;
    CURSOR_Y = 28;

    char login[64] = {'\n'};
    char password[64] = {'\n'};

    AuthHandler(login, password);
    system("clear");
    MainHandler(login, password);
    system("clear");    
    pthread_mutex_destroy(&CURSOR_MUTEX);
}
