/*
** Usei como tutorial do minilibx a seguinte documentação criada por cadetes:
** https://harm-smits.github.io/42docs/libs/minilibx
**
**
** Contém keycodes de todas as teclas, até de outros alfabetos, mas parece não
** funcionar quando incluído. Ainda serve pra pesquisar e encontrar os códigos
** de teclas:
** #include <keysymdef.h>
**
**
** A lib math é permitida no projeto, e deve ser linkada com a opção -lm vista
** no script de compilação comp.sh. A função printf também é permitida.
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "mlx.h"

/*
** Keycodes que encontrei no header keysymdef.h, com exceção dos botões do mouse
** que parecem seguir a lógica: botão 1 = 1; botão 2 = 2; etc.
*/
#define KEY_C	0x63
#define KEY_F	0x66
#define KEY_ESC	0xff1b
#define MOUSE_1	1

/*
** Typedefs que acho que serão ao menos um pouco úteis pra deixar o programa
** mais legível.
*/
typedef unsigned char	t_color;
typedef int				t_rgb;

/*
** Um struct para guardar endereços relativos a uma janela. mlx para guardar
** a inicialização do mlx, e ptr para guardar o endereço de uma janela criada.
** Talvez seja mais sensato juntar esse struct com o próximo abaixo.
*/
typedef struct	s_window
{
	void	*mlx;
	void	*ptr;
}				t_window;

/*
** Um struct para guardar informações úteis a uma imagem que será composta e
** então exibida na janela. "ptr" é um ponteiro para a própria imagem, enquanto
** "addr" serve para alinhar a impressão de pixels de acordo com sua imagem.
** As demais variáveis são usadas para a própria impressão ou para alinhamento.
** Mais sobre isso na função create_image.
*/
typedef struct	s_image
{
	void	*ptr;
	char	*addr;
	int		width;
	int		height;
	int		bits_per_pixel;
	int		line_len;
	int		endian;
}				t_image;

/*
** Um struct que penso que será útil na parte de ray tracing, sabendo que
** a diferença entre dois pontos (ponto1 - ponto2) caracteriza um vetor,
** no sentido geométrico da palavra.
*/
typedef struct	s_coord
{
	double		x;
	double		y;
	double		z;
}				t_coord;

/*
** Uma função para compor uma variável int que representa uma cor rgb no mlx,
** a partir de inputs básicos do "código" da cor, sendo que cada argumento
** aceita valores no range de unsigned char (0-255). Talvez seja necessário
** adicionar um argumento a mais para representar transparência ou opacidade.
*/
t_rgb	color_picker(t_color red, t_color green, t_color blue)
{
	return (red << 16 | green << 8 | blue);
}

/*
** Cria uma imagem vazia, que será composta e então exibida em uma janela.
** O endereço da imagem é guardado em img->ptr. A função mlx_get_data_addr
** determina o tal "alinhamento" para composição da imagem, recebendo ponteiros
** para as variáveis bits_per_pixel, line_len, e endian, cujos valores serão
** atribuídos pela própria função. Tendo estas informação, é possível compor
** a imagem com a certeza de que as linhas impressas terão a mesma largura da
** imagem, por exemplo.
*/
void	create_image(t_image *img, void *mlx, int width, int height)
{
	img->ptr = mlx_new_image(mlx, width, height);
	img->addr = mlx_get_data_addr(img->ptr, &img->bits_per_pixel,
												&img->line_len, &img->endian);
	img->width = width;
	img->height = height;
}

/*
** Desenha um único pixel de cor "color" nas coordenadas x e y da imagem "img".
** Primeiro se encontra a posição do pixel dentro da imagem usando as variáveis
** coletadas na função acima, e então é atribuída a este pixel a cor recebida
** como argumento.
**
** Essa função apenas desenha o pixel na imagem, mas não o força a ser exibido
** na janela imediatamente. A função padrão mlx_pixel_put faria exatamente isso
** (desenhar e exibir), o que a faz ser muito lenta. Quando testei, podia ver
** a imagem sendo preenchida linha por linha. Compor toda a imagem e então
** exibí-la parece mais útil.
*/
void	draw_pixel(t_image *img, int x, int y, int color)
{
	char	*pix;

	pix = img->addr + (y * img->line_len + x * (img->bits_per_pixel / 8));
	*(unsigned int*)pix = color;
}

/*
** Preenche uma imagem "img" inteira com a cor "color", pixel por pixel,
** criando um fundo.
*/
void	fill_image(t_image *img, t_rgb color)
{
	int	x;
	int	y;

	x = 0;
	y = 0;
	while (y <= img->height)
	{
		while (x <= img->width)
		{
			draw_pixel(img, x, y, color);
			x++;
		}
		y++;
		x = 0;
	}
}

/*
** Desenha no centro imagem "img" um quadrado de lado "side" e cor "color".
*/
void	draw_square(t_image *img, int side, t_rgb color)
{
	int	x;
	int	y;

	x = (img->width - side) / 2;
	y = (img->height - side) / 2;
	while (y <= (img->height + side) / 2)
	{
		while (x <= (img->width + side) / 2)
		{
			draw_pixel(img, x, y, color);
			x++;
		}
		y++;
		x = (img->width - side) / 2;
	}
}

/*
** Desenha no centro da imagem um círculo de raio "radius" e cor "color".
** Ainda longe de qualquer coisa sobre ray tracing, mas começamos a mexer
** com alguma geometria. A condição if especifica que um pixel só será desenhado
** se estiver dentro da área delimitada pelo raio do círculo, usando a
** equação:
**
** 		(x - a)² + (y - b)² <= R²
**
** Sendo:
** 		x e y as variáveis sendo iteradas.
** 		a e b as coordenadas do centro do círculo.
** 		R o raio do círculo.
*/
void	draw_circle(t_image *img, int radius, t_rgb color)
{
	int	radius_squared;
	int	center_x;
	int	center_y;
	int	x;
	int	y;

	radius_squared = pow(radius, 2);
	center_x = img->width / 2;
	center_y = img->height / 2;
	x = center_x - radius;
	y = center_y - radius;
	while (y <= center_y + radius)
	{
		while (x <= center_x + radius)
		{
			if ((pow(x - center_x, 2) + pow(y - center_y, 2)) <= radius_squared)
				draw_pixel(img, x, y, color);
			x++;
		}
		y++;
		x = center_x - radius;
	}
}

int		close_window(t_window *window)
{
	mlx_destroy_window(window->mlx, window->ptr);
	window->ptr = NULL;
	exit(1);
	return (0);
}

/*
** A função mlx_key_hook, chamada na main, especifica que a cada loop do mlx
** será chamada esta função keyboard_input que recebe como argumento uma tecla
** pressionada no teclado. Se essa tecla for Esc, o programa se encerra.
** Por enquanto esse encerramento exibe erros, provavelmente por alguma outra
** função ainda tentar usar o ponteiro window->mlx depois dele deixar de ser
** válido. Mais explicações sobre hooks na função main.
*/
int		keyboard_input(int keycode, t_window *window)
{
	if (keycode == KEY_ESC)
		close_window(window);
	return (0);
}

/*
** A função mlx_mouse_hook, chamado na main, especifica que a cada loop do mlx
** será chamada esta função mouse_track, que ao receber o input MOUSE_1 (botão
** esquerdo do mouse) DENTRO da janela de renderização, serão impressas no
** terminal as coordenadas de onde o cursor clicou dentro da janela. Mais sobre
* hooks na função main.
*/
int		mouse_track(int button, int x, int y, t_window *window)
{
	if (button == MOUSE_1)
	{
		mlx_mouse_get_pos(window->mlx, window->ptr, &x, &y);
		printf("Cursor coords.: %4d x %4d\n", x, y);
	}
	return (0);
}

int		main(void)
{
	t_window	window;
	t_image		image;
	int			width = 800;
	int			height = 600;

	window.mlx = mlx_init();
	window.ptr = mlx_new_window(window.mlx, width, height, "DON'T PANIC");
	/*
	** Acima, inicializa o mlx e cria uma nova janela de dimensões
	** width x height, com o título "DON'T PANIC". No ponteiro mlx guardamos a
	** referência da instância do mlx que usaremos, e em window.ptr guardamos
	** a referência da janela criada.
	*/

	create_image(&image, window.mlx, width, height);
	fill_image(&image, color_picker(15, 20, 25));
	draw_square(&image, 500, color_picker(255, 255, 255));
	draw_circle(&image, 200, color_picker(64, 128, 196));
	mlx_put_image_to_window(window.mlx, window.ptr, image.ptr, 0, 0);
	/*
	** Acima, cria uma nova imagem, a preenche com cinza escuro, desenha um
	** quadrado de lado 500 pixels e cor branca, desenha um círculo de raio 200
	** pixels de cor azul, e por fim exibe esta imagem na janela criada no bloco
	** anterior.
	*/

	mlx_hook(window.ptr, 33, 1L << 17, close_window, &window);
	mlx_mouse_hook(window.ptr, mouse_track, &window);
	mlx_key_hook(window.ptr, keyboard_input, &window);
	mlx_loop(window.mlx);
	/*
	** Uma vez chamada a função mlx_loop no fim da função, teremos controle
	** sobre o programa através apenas de hooks, pelo que entendi. A cada
	** "iteração" do mlx_loop, serão chamados os hooks definidos antes do
	** mlx_loop no seu programa.
	**
	** No meu caso, temos um hook do mouse e outro do teclado, e a partir deles
	** são chamadas as funções mouse_track e keyboard_input. Nesses dois hooks,
	** o argumento do meio é um ponteiro para função com retorno int. Porém os
	** argumentos das funções indicadas pelos ponteiros dependem do tipo de hook
	** por onde estão sendo chamadas. Para saber quais são os argumentos, pode
	** se usar o manual do mlx ou o link no topo do arquivo.
	*/
	return (0);
}
