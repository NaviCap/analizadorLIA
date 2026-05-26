// Noe Ezequiel Rodriguez Dominguez
// Jesus Ivan Del Campo Pena

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include <windows.h>
#include <commdlg.h>


// Columnas de la matriz de transiciones 
// a mi entender tuve que hacer un ajuste en el caso del automata y matriz para el salto de linea y comentario, haciendo una colmna propia para el salto de linea

#define COL_MINUS    0   // a-z (excepto e)
#define COL_MAYUS    1   // A-Z (excepto E)
#define COL_DIGITO   2   // 0-9
#define COL_PUNTO    3   // .
#define COL_E        4   // E o e
#define COL_MAS      5   // +
#define COL_MENOS    6   // -
#define COL_MULT     7   // *
#define COL_DIV      8   // /
#define COL_IGUAL    9   // =
#define COL_MENOR    10  // 
#define COL_MAYOR    11  // >
#define COL_NOT      12  // !
#define COL_AMP      13  // &
#define COL_PIPE     14  // |
#define COL_LPAREN   15  // (
#define COL_RPAREN   16  // )
#define COL_LBRACK   17  // [
#define COL_RBRACK   18  // ]
#define COL_LBRACE   19  // {
#define COL_RBRACE   20  // }
#define COL_SEMICOL  21  // ;
#define COL_COMA     22  // ,
#define COL_COMILLA  23  // '
#define COL_DQUOTE   24  // "
#define COL_DOLAR    25  // $  
#define COL_GUION    26  // _
#define COL_PORCENT  27  // %
#define COL_DOSPUNT  28  // :
#define COL_DELIM    29  // espacio | \t | \r
#define COL_NEWLINE  30  // \n  mencion del salto de linea para la funcion de q19 y para que el automata lo distinga de los delimitadores, ademas de que en q0 se regrese a 0 y no se emita token
#define COL_OTRO     31  // 

#define NUM_COLS    32
#define NUM_ESTADOS 21

// ESTRUCTURA TOKEN
struct Token {
    std::string lexema;
    std::string gramema;
    int         linea;
    bool        es_error;
    std::string mensaje_error;
};


// CLASE AnalizadorLexico

class AnalizadorLexico {
public:
    AnalizadorLexico();
    std::vector<Token> analizar(const std::string& codigo);

public:
    int tabla_col[256];
    int M[NUM_ESTADOS][NUM_COLS];

    void        inicializar_relaciona();
    void        inicializar_matriz();
    int         relaciona(unsigned char c);
    std::string obtener_gramema(int estado, const std::string& lexema);
    std::string obtener_error(int estado);
    std::string buscar_reservada(const std::string& lexema);
};


// Palabras reservadas como menciona el automata, unicamente las palabras reservadas siempre iniciaran en minusculas

struct PalabraReservada { const char* palabra; const char* gramema; };

static PalabraReservada reservadas[] = {
    {"class",    "palabra reservada"},
    {"endclass", "palabra reservada"},
    {"int",      "palabra reservada"},
    {"float",    "palabra reservada"},
    {"char",     "palabra reservada"},
    {"string",   "palabra reservada"},
    {"bool",     "palabra reservada"},
    {"if",       "palabra reservada"},
    {"else",     "palabra reservada"},
    {"do",       "palabra reservada"},
    {"while",    "palabra reservada"},
    {"input",    "palabra reservada"},
    {"output",   "palabra reservada"},
    {"def",      "palabra reservada"},
    {"to",       "palabra reservada"},
    {"break",    "palabra reservada"},
    {"loop",     "palabra reservada"},
    {nullptr, nullptr}
};


// Gramemas (indice a partir de codigo - 100)

static const char* gramemas[] = {
    "identificador (minuscula)",     // 100
    "identificador (mayuscula)",     // 101
    "constante entera",              // 102
    "constante real",                // 103
    "constante notacion cientifica", // 104
    "suma",                          // 105
    "resta",                         // 106
    "multiplicacion",                // 107
    "division",                      // 108
    "asignacion",                    // 109
    "igual",                         // 110
    "menor que",                     // 111
    "menor o igual",                 // 112
    "mayor que",                     // 113
    "mayor o igual",                 // 114
    "not",                           // 115
    "diferente",                     // 116
    "and",                           // 117
    "or",                            // 118
    "parentesis abre",               // 119
    "parentesis cierra",             // 120
    "corchete abre",                 // 121
    "corchete cierra",               // 122
    "punto y coma",                  // 123
    "coma",                          // 124
    "constante caracter",            // 125
    "constante string",              // 126
    "comentario de linea",           // 127
    "modulus",                       // 128
    "llave abre",                    // 129
    "llave cierra",                  // 130
    "dos puntos"                     // 131
};


// Mensajes de error (indice a partir de codigo - 500)

static const char* mensajes_error[] = {
    "500: Elemento no reconocido por el lenguaje",
    "501: Constante real mal formada, se esperaban digitos tras el punto",
    "502: Notacion cientifica mal formada, se esperaba digito, + o -",
    "503: Operador & incompleto, se esperaba &&",
    "504: Operador | incompleto, se esperaba ||",
    "505: Constante caracter mal formada",
    "506: Constante string mal formada, falta cierre",
    "507: Caracter no pertenece al lenguaje LIA"
};

// Constructor

AnalizadorLexico::AnalizadorLexico() {
    inicializar_relaciona();
    inicializar_matriz();
}


void AnalizadorLexico::inicializar_relaciona() {
    for (int i = 0; i < 256; i++) tabla_col[i] = COL_OTRO;

    for (int c = 'a'; c <= 'z'; c++) tabla_col[c] = COL_MINUS;
    for (int c = 'A'; c <= 'Z'; c++) tabla_col[c] = COL_MAYUS;
    for (int c = '0'; c <= '9'; c++) tabla_col[c] = COL_DIGITO;

    tabla_col['e'] = COL_E;       tabla_col['E'] = COL_E;
    tabla_col['.'] = COL_PUNTO;   tabla_col['+'] = COL_MAS;
    tabla_col['-'] = COL_MENOS;   tabla_col['*'] = COL_MULT;
    tabla_col['/'] = COL_DIV;     tabla_col['='] = COL_IGUAL;
    tabla_col['<'] = COL_MENOR;   tabla_col['>'] = COL_MAYOR;
    tabla_col['!'] = COL_NOT;     tabla_col['&'] = COL_AMP;
    tabla_col['|'] = COL_PIPE;    tabla_col['('] = COL_LPAREN;
    tabla_col[')'] = COL_RPAREN;  tabla_col['['] = COL_LBRACK;
    tabla_col[']'] = COL_RBRACK;  tabla_col['{'] = COL_LBRACE;
    tabla_col['}'] = COL_RBRACE;  tabla_col[';'] = COL_SEMICOL;
    tabla_col[','] = COL_COMA;    tabla_col['\''] = COL_COMILLA;
    tabla_col['"'] = COL_DQUOTE;  tabla_col['$'] = COL_DOLAR;
    tabla_col['_'] = COL_GUION;   tabla_col['%'] = COL_PORCENT;
    tabla_col[':'] = COL_DOSPUNT;
    tabla_col[' '] = COL_DELIM;
    tabla_col['\t'] = COL_DELIM;
    tabla_col['\r'] = COL_DELIM;
    tabla_col['\n'] = COL_NEWLINE;
}


void AnalizadorLexico::inicializar_matriz() {
    int temp[NUM_ESTADOS][NUM_COLS] = {
        //         l    L    d    .    E    +    -    *    /    =    <    >    !    &    |    (    )    [    ]    {    }    ;    ,    '    "    $    _    %    :   SP   \n  otro
        /* q0  */ {  1,   2,   3, 507,   1, 105, 106, 107, 108,   9,  10,  11,  12,  13,  14, 119, 120, 121, 122, 129, 130, 123, 124,  15,  17,  19,   1, 128, 131,   0,   0, 507},
        /* q1  */ {  1,   1,   1, 100,   1, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,   1, 100, 100, 100, 100, 100},
        /* q2  */ {  2,   2,   2, 101,   2, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101,   2, 101, 101, 101, 101, 101},
        /* q3  */ {500, 500,   3,   4, 500, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 500, 102, 102, 102, 102, 500},
        /* q4  */ {501, 501,   5, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501, 501},
        /* q5  */ {501, 501,   5, 501,   6, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 501, 103, 103, 103, 103, 501},
        /* q6  */ {502, 502,   8, 502, 502,   7,   7, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502},
        /* q7  */ {502, 502,   8, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502, 502},
        /* q8  */ {507, 507,   8, 507, 507, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 507, 104, 104, 104, 104, 507},
        /* q9  */ {109, 109, 109, 109, 109, 109, 109, 109, 109, 110, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109},
        /* q10 */ {111, 111, 111, 111, 111, 111, 111, 111, 111, 112, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111},
        /* q11 */ {113, 113, 113, 113, 113, 113, 113, 113, 113, 114, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113},
        /* q12 */ {115, 115, 115, 115, 115, 115, 115, 115, 115, 116, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115},
        /* q13 */ {503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 117, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503, 503},
        /* q14 */ {504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 118, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504, 504},
        /* q15 */ { 16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16, 505,  16,  16,  16,  16,  16, 505, 505, 505},
        /* q16 */ {505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 505, 125, 505, 505, 505, 505, 505, 505, 505, 505},
        /* q17 */ { 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18, 126,  18,  18,  18,  18,  18,  18,  18},
        /* q18 */ { 18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,  18, 126,  18,  18,  18,  18,  18,  18,  18},
        /* q19 */ { 19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,  19, 127,  19},
        /* q20 */ {108, 108, 108, 108, 108, 108, 108, 108,  19, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108}
    };
    memcpy(M, temp, sizeof(M));
}

int AnalizadorLexico::relaciona(unsigned char c) {
    return tabla_col[c];
}

std::string AnalizadorLexico::buscar_reservada(const std::string& lexema) {
    for (int i = 0; reservadas[i].palabra != nullptr; i++)
        if (lexema == reservadas[i].palabra)
            return reservadas[i].gramema;
    return "";
}

std::string AnalizadorLexico::obtener_gramema(int estado, const std::string& lexema) {
    if (estado == 100 || estado == 101) {
        std::string pr = buscar_reservada(lexema);
        if (!pr.empty()) return pr;
        return gramemas[estado - 100];
    }
    if (estado >= 100 && estado <= 131)
        return gramemas[estado - 100];
    return "desconocido";
}

std::string AnalizadorLexico::obtener_error(int estado) {
    if (estado >= 500 && estado <= 507)
        return mensajes_error[estado - 500];
    return "Error desconocido (estado " + std::to_string(estado) + ")";
}


std::vector<Token> AnalizadorLexico::analizar(const std::string& codigo) {
    std::vector<Token> tokens;
    int pos = 0;
    int linea = 1;
    int largo = (int)codigo.size();

    while (pos <= largo) {
        int         edo = 0;
        std::string lexema = "";

        while (edo < 100) {
            unsigned char car;
            if (pos == largo) {
                car = '\n';
            }
            else if (pos > largo) {
                break;
            }
            else {
                car = (unsigned char)codigo[pos];
            }

            int col = relaciona(car);
            int sig = M[edo][col];

            if (sig >= 100) {
                bool es_delimitador = (sig == 100 || sig == 101 ||
                    sig == 102 || sig == 103 ||
                    sig == 104);
                if (!es_delimitador) {
                    lexema += car;
                    if (pos < largo) pos++;
                    else pos = largo + 1;
                    if (car == '\n') linea++;
                }
                edo = sig;
            }
            else {
                if (sig != 0) lexema += car;
                if (pos < largo) pos++;
                else pos = largo + 1;
                if (car == '\n') linea++;
                edo = sig;
            }
        }

        if (edo < 100 && !lexema.empty()) {
            Token t;
            t.lexema = lexema;
            t.linea = linea;
            t.es_error = true;
            t.gramema = "ERROR";
            t.mensaje_error = "500: Elemento no reconocido por el lenguaje | cerca de: '" + lexema + "'";
            tokens.push_back(t);
            continue;
        }

        if (lexema.empty() && edo == 0) continue;

        Token t;
        t.lexema = lexema;
        t.linea = linea;

        if (edo >= 100 && edo <= 199) {
            t.es_error = false;
            t.gramema = obtener_gramema(edo, lexema);
        }
        else if (edo >= 500) {
            t.es_error = true;
            t.gramema = "ERROR";
            t.mensaje_error = obtener_error(edo) + " | cerca de: '" + lexema + "'";
        }

        tokens.push_back(t);
    }

    return tokens;
}


// leer_archivo

std::string leer_archivo(const std::string& nombre) {
    std::ifstream f(nombre);
    if (!f.is_open())
        throw std::runtime_error("no se pudo abrir el archivo '" + nombre + "'");
    std::stringstream ss;
    ss << f.rdbuf();
    if (f.bad())
        throw std::runtime_error("error de lectura en el archivo '" + nombre + "'");
    return ss.str();
}


// INTERFAZ GRAFICA WIN32

#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#pragma comment(lib, "Comdlg32.lib")

#define ID_TXT_CODIGO  1001
#define ID_TXT_TOKENS  1002
#define ID_TXT_SINTAX  1003
#define ID_TXT_ERRORES 1004

#define ID_BTN_ABRIR    2001
#define ID_BTN_GUARDAR  2002
#define ID_BTN_LIMPIAR  2003
#define ID_BTN_ANALIZAR 2004
#define ID_BTN_SALIR    2005

HWND hCodigo, hTokens, hSintaxis, hErrores;
HWND hBtnAbrir, hBtnGuardar, hBtnLimpiar, hBtnAnalizar, hBtnSalir;
HFONT gFuenteNormal = NULL;
std::string archivoActual = "";

std::string obtenerTexto(HWND hEdit) {
    int len = GetWindowTextLengthA(hEdit);
    if (len == 0) return "";
    std::string texto(len + 1, '\0');
    GetWindowTextA(hEdit, &texto[0], len + 1);
    texto.resize(len);
    return texto;
}

void ponerTexto(HWND hEdit, const std::string& texto) {
    SetWindowTextA(hEdit, texto.c_str());
}
// permite leer archivos .lia como fue solicitado, aunque tambien agregue la opcion de .txt meramente por pruebas, asi que tambien es valido
void abrirArchivo(HWND hwnd) {
    char nombre[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = nombre;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Archivos LIA (*.lia)\0*.lia\0Archivos de texto (*.txt)\0*.txt\0Todos (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        try {
            std::string contenido = leer_archivo(nombre);
            std::string convertido;
            convertido.reserve(contenido.size());
            for (size_t i = 0; i < contenido.size(); i++) {
                if (contenido[i] == '\r') continue;
                if (contenido[i] == '\n') convertido += '\r';
                convertido += contenido[i];
            }
            ponerTexto(hCodigo, convertido);
            archivoActual = nombre;
        }
        catch (const std::exception& e) {
            MessageBoxA(hwnd, e.what(), "Error al abrir", MB_ICONERROR);
        }
    }
}
//en este caso siempre creara los archivos .lia ya que eso fue lo solicitado 
void guardarArchivo(HWND hwnd) {
    char nombre[MAX_PATH] = "";
    if (!archivoActual.empty())
        strncpy_s(nombre, archivoActual.c_str(), MAX_PATH - 1);

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = nombre;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "Archivos LIA (*.lia)\0*.lia\0Archivos de texto (*.txt)\0*.txt\0Todos (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "lia";

    if (GetSaveFileNameA(&ofn)) {
        std::ofstream f(nombre);
        if (!f.is_open()) {
            MessageBoxA(hwnd, "No se pudo guardar el archivo.", "Error", MB_ICONERROR);
            return;
        }
        std::string texto = obtenerTexto(hCodigo);
        for (size_t i = 0; i < texto.size(); i++) {
            if (texto[i] == '\r') continue;
            f << texto[i];
        }
        archivoActual = nombre;
        MessageBoxA(hwnd, "Archivo guardado correctamente.", "Guardar", MB_ICONINFORMATION);
    }
}

void limpiarTodo() {
    ponerTexto(hCodigo, "");
    ponerTexto(hTokens, "");
    ponerTexto(hSintaxis, "");
    ponerTexto(hErrores, "");
    archivoActual = "";
}

void analizarCodigo() {
    std::string codigo = obtenerTexto(hCodigo);

    AnalizadorLexico lexico;
    std::vector<Token> lista = lexico.analizar(codigo);

    std::stringstream ssTokens;
    std::stringstream ssErrores;
    int totalTokens = 0;
    int totalErrores = 0;

    ssTokens << "Lexema\t\t\tGramema\r\n";
    ssTokens << "--------------------------------------------------\r\n";

    for (const Token& t : lista) {
        if (t.lexema.empty()) continue;
        if (t.es_error) {
            ssErrores << "[Linea " << t.linea << "] " << t.mensaje_error << "\r\n";
            totalErrores++;
        }
        else {
            ssTokens << t.lexema << "\t\t\t" << t.gramema << "\r\n";
            totalTokens++;
        }
    }

    ssTokens << "-------------------------------\r\n";
    ssTokens << "Total tokens: " << totalTokens << "\r\n";

    if (totalErrores == 0) {
        ssErrores << "OK - Sin errores lexicos.\r\n";
    }
    else {
        ssErrores << "-------------------------\r\n";
        ssErrores << "Total errores: " << totalErrores << "\r\n";
    }
// como no tenemos aun el analizador sintactico no lo agregue
    std::stringstream ssSintaxis;
    if (totalErrores == 0) {
       
        ssSintaxis << "Analisis sintactico no disponible.\r\n";
    }
    else {
        ssSintaxis << "Analisis sintactico no disponible.\r\n";
    }

    ponerTexto(hTokens, ssTokens.str());
    ponerTexto(hSintaxis, ssSintaxis.str());
    ponerTexto(hErrores, ssErrores.str());
}

void aplicarFuente(HWND h, HFONT f) {
    SendMessageA(h, WM_SETFONT, (WPARAM)f, TRUE);
}

void crearControles(HWND hwnd) {
    gFuenteNormal = CreateFontA(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas"
    );

    CreateWindowA("STATIC", "Codigo fuente",
        WS_CHILD | WS_VISIBLE,
        20, 10, 150, 22, hwnd, NULL, NULL, NULL);

    hCodigo = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        20, 35, 610, 450,
        hwnd, (HMENU)ID_TXT_CODIGO, NULL, NULL);

    CreateWindowA("STATIC", "Tokens",
        WS_CHILD | WS_VISIBLE,
        650, 10, 120, 22, hwnd, NULL, NULL, NULL);

    hTokens = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        650, 35, 320, 195,
        hwnd, (HMENU)ID_TXT_TOKENS, NULL, NULL);

    CreateWindowA("STATIC", "Sintaxis",
        WS_CHILD | WS_VISIBLE,
        650, 240, 120, 22, hwnd, NULL, NULL, NULL);

    hSintaxis = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        650, 265, 320, 110,
        hwnd, (HMENU)ID_TXT_SINTAX, NULL, NULL);

    CreateWindowA("STATIC", "Errores",
        WS_CHILD | WS_VISIBLE,
        650, 385, 120, 22, hwnd, NULL, NULL, NULL);

    hErrores = CreateWindowExA(
        WS_EX_CLIENTEDGE, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        650, 410, 320, 75,
        hwnd, (HMENU)ID_TXT_ERRORES, NULL, NULL);

    hBtnAbrir = CreateWindowA("BUTTON", "Abrir",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        20, 505, 105, 35, hwnd, (HMENU)ID_BTN_ABRIR, NULL, NULL);

    hBtnGuardar = CreateWindowA("BUTTON", "Guardar",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        140, 505, 105, 35, hwnd, (HMENU)ID_BTN_GUARDAR, NULL, NULL);

    hBtnLimpiar = CreateWindowA("BUTTON", "Limpiar",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        260, 505, 105, 35, hwnd, (HMENU)ID_BTN_LIMPIAR, NULL, NULL);

    hBtnAnalizar = CreateWindowA("BUTTON", "Analizar",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        380, 505, 105, 35, hwnd, (HMENU)ID_BTN_ANALIZAR, NULL, NULL);

    hBtnSalir = CreateWindowA("BUTTON", "Salir",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        500, 505, 105, 35, hwnd, (HMENU)ID_BTN_SALIR, NULL, NULL);

    aplicarFuente(hCodigo, gFuenteNormal);
    aplicarFuente(hTokens, gFuenteNormal);
    aplicarFuente(hSintaxis, gFuenteNormal);
    aplicarFuente(hErrores, gFuenteNormal);
    aplicarFuente(hBtnAbrir, gFuenteNormal);
    aplicarFuente(hBtnGuardar, gFuenteNormal);
    aplicarFuente(hBtnLimpiar, gFuenteNormal);
    aplicarFuente(hBtnAnalizar, gFuenteNormal);
    aplicarFuente(hBtnSalir, gFuenteNormal);
}

LRESULT CALLBACK ventanaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        crearControles(hwnd);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_ABRIR:    abrirArchivo(hwnd);  break;
        case ID_BTN_GUARDAR:  guardarArchivo(hwnd); break;
        case ID_BTN_LIMPIAR:  limpiarTodo();        break;
        case ID_BTN_ANALIZAR: analizarCodigo();     break;
        case ID_BTN_SALIR:    PostQuitMessage(0);   break;
        }
        break;
    case WM_DESTROY:
        if (gFuenteNormal) DeleteObject(gFuenteNormal);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int main() {
    HINSTANCE hInstancia = GetModuleHandleA(NULL);

    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = ventanaProc;
    wc.hInstance = hInstancia;
    wc.lpszClassName = "VentanaLIA";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "No se pudo registrar la ventana.", "Error", MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowA(
        "VentanaLIA",
        "Proyecto LIA - Analizador Lexico",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1010, 600,
        NULL, NULL, hInstancia, NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "No se pudo crear la ventana.", "Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}
