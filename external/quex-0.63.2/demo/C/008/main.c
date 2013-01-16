#include <stdio.h>
#include "Calc_parser.tab.h"
#include "Calc_lexer.h"

int Calc_yyparse(quex_Calc_lexer  *qlex);

int main(int argc, char** argv) 
{
	quex_Calc_lexer qlex;
    
    QUEX_NAME(construct_file_name)(&qlex, argc == 1 ? "example.txt" : argv[1], 0x0, false);

    printf("Calculator Example Application\n");
    printf("Contributed by: Marco Antonelli (date: 09y11m7d)\n\n");

	int ret = Calc_yyparse(&qlex);
	if (ret!=0)
	{
		printf("Some error in yyparse\n");
		return ret;
	}

    QUEX_NAME(destruct)(&qlex);
	return 0;
}

//void printit(wstring *arg)
//{
//	
//	const wchar_t *str = arg->c_str();
//	char * dest;
//	size_t len;
//	
//	/* first arg == NULL means 'calculate needed space' */
//	len = wcstombs(NULL, str, 0);
//	
//	/* a size of -1 means there are characters that could not
//	be converted to current locale */
//	if(len == (size_t)-1)
//	{
//		cout << "wchar print error" << endl;
//		return;
//	}
//	
//	/* malloc the necessary space */
//	if((dest = (char *)malloc(len + 1)) == NULL)
//	{
//		cout << "malloc error" << endl;
//		return;
//	}
//	
//	/* really do it */
//	wcstombs(dest, str, len);
//	
//	/* ensure NULL-termination */
//	dest[len] = '\0';
//	
//	cout << "value: " << dest << endl;
//	
//	free(dest);
//}


