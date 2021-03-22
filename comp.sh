#!/bin/sh

# Esse método de compilação tem funcionado no meu Linux nativo, mas ainda não
# testei na VM da 42. O certo é que serão necessárias no mínimo as dependências
# xorg e libxext-dev. Para mais detalhes, veja o link:
# https://harm-smits.github.io/42docs/libs/minilibx/getting_started.html#compilation-on-linux

clang -Wall -Werror -Wextra -fsanitize=address -O3 main.c -I. -L. -lmlx -lXext -lX11 -lm -o mlx_test && ./mlx_test
