#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <locale.h>
#define DEEP 1
typedef enum {
	WHITE = 1,
	BLACK = -1
} b_w;

typedef struct {
	int desk[24];
	b_w cur_player;
	int dice[2];
	bool used_dice[2];
	bool head_used;
	bool first_move_w;
	bool first_move_b;
	int aviable_moves[60][3];	//нач. позиция, кон. позиция, номер кубика
	int white_dropped;
	int black_dropped;
}table_s;

typedef struct {
	int from;
	int to;
	int dice;
	int index;
	double score;
}move;

table_s table;

void reset_table() {
	table.head_used = false;
	table.used_dice[0] = false;
	table.used_dice[1] = false;
	for (int i = 0; i < 60; i++) {
		table.aviable_moves[i][0] = 0;
		table.aviable_moves[i][1] = 0;
		table.aviable_moves[i][2] = 0;
	}
}

void init_table() {
	table.white_dropped = 0;
	table.black_dropped = 0;
	table.first_move_w = true;
	table.first_move_b = true;
	table.desk[0] = 15;
	table.desk[12] = -15;
	table.cur_player = WHITE;
	reset_table();
}

char print_helper(int i) {
	if (table.desk[i] > 0)
		return 'W';
	else if (table.desk[i] < 0)
		return 'B';
	else
		return ' ';
}

void print_desk() {
	printf("\n----------------------------------------------------------------------------------------------------\n");

	for (int i = 11; i >= 0; i--) {
		printf("%d%c\t", abs(table.desk[i]), print_helper(i));
	}
	printf("\n");
	for (int i = 12; i < 24; i++) {
		printf("%d%c\t", abs(table.desk[i]), print_helper(i));
	}

	printf("\n----------------------------------------------------------------------------------------------------\n");

}

int cmp(const void* a, const void* b) {
	double diff = ((move*)b)->score - ((move*)a)->score;
	return (diff > 0) ? 1:-1;  // по убыванию: лучшие ходы первыми
}
void roll_dice() {
	table.dice[0] = 1 + rand() % 6;
	table.dice[1] = 1 + rand() % 6;
}

bool all_home(table_s table) {	//все фишки для текущего цвета в доме
	if (table.cur_player == WHITE) {
		for (int i = 0; i < 18; i++)
			if (table.desk[i] > 0) return false;	//если хоть одна белая стоит вне дома -> false
	}
	else {
		for (int i = 0; i < 6; i++)
			if (table.desk[i] < 0) return false;
		for (int i = 12; i < 24; i++)
			if (table.desk[i] < 0) return false;
	}
	return true;
}

bool check_fence(table_s table, int pos) {
	int cnt = 1;
	int calc_cur_pos;
	for (int i = 1; i <= 5; i++) {
		calc_cur_pos = pos + i;
		if ((table.cur_player == WHITE && table.desk[calc_cur_pos % 24] <= 0) || (table.cur_player == BLACK && table.desk[calc_cur_pos % 24] >= 0)) {
			break;
		}
		cnt++;
	}
	for (int i = 1; i <= 5; i++) {
		int calc_cur_pos = pos - i;
		if (calc_cur_pos < 0)
			calc_cur_pos += 24;
		if ((table.cur_player == WHITE && table.desk[calc_cur_pos] <= 0) || (table.cur_player == BLACK && table.desk[calc_cur_pos] >= 0)) {
			break;
		}
		cnt++;
	}
	if (cnt >= 6) {
		return false;
	}
	return true;
}

int all_moves(table_s* table) {
	int count_moves = 0;
	int limit = 2;
	if (table->dice[0] == table->dice[1])	//если дубль
		limit = 1;

	for (int move_i = 0; move_i < limit; move_i++) {
		if (table->used_dice[move_i])
			continue;
		if (!all_home(*table)) {
			if (table->cur_player == WHITE) {
				for (int pos = 0; pos < 24; pos++) {
					if (table->desk[pos] > 0 && table->desk[pos + table->dice[move_i]] >= 0 && (pos + table->dice[move_i]) < 24 && (!(pos == 0 && table->head_used))) {
						table->desk[pos]--;
						if (check_fence(*table, pos + table->dice[move_i])) {
							table->aviable_moves[count_moves][0] = pos;
							table->aviable_moves[count_moves][1] = pos + table->dice[move_i];
							table->aviable_moves[count_moves][2] = move_i;
							count_moves++;
						}
						table->desk[pos]++;
					}
				}
			}
			else if (table->cur_player == BLACK) {
				int des_move;
				for (int pos = 0; pos < 24; pos++) {
					if (table->desk[pos] < 0 && (!(pos == 12 && table->head_used))) {
						des_move = pos + table->dice[move_i];
						if (des_move >= 24)
							des_move -= 24;
						if (table->desk[des_move] <= 0 && !(pos >= 0 && pos < 12 && des_move > 11)) {
							table->desk[pos]++;
							if (check_fence(*table, des_move)) {
								table->aviable_moves[count_moves][0] = pos;
								table->aviable_moves[count_moves][1] = des_move;
								table->aviable_moves[count_moves][2] = move_i;
								count_moves++;
							}
							table->desk[pos]--;
						}
					}
				}
			}
		}
		else {	//когда фишки в доме
			if (table->cur_player == WHITE) {
				int cur_pos = 24 - table->dice[move_i];
				if (table->desk[cur_pos] > 0) {
					//выбрасываем
					table->aviable_moves[count_moves][0] = cur_pos;
					table->aviable_moves[count_moves][1] = -1;
					table->aviable_moves[count_moves][2] = move_i;
					count_moves++;
				}
				else {
					bool find_flag = false;
					for (int i = cur_pos; i >= 18; i--) {	//поиск хода внутри дома
						if (table->desk[i] > 0 && table->desk[i + table->dice[move_i]] >= 0 && (i + table->dice[move_i]) < 24) {
							table->aviable_moves[count_moves][0] = i;
							table->aviable_moves[count_moves][1] = i + table->dice[move_i];
							table->aviable_moves[count_moves][2] = move_i;
							count_moves++;
							find_flag = 1;
						}
					}
					if (!find_flag) {	//выброс с самого близкого занятого пункта
						int far_cell = -1;
						for (int i = 23; i >= 18; i--) {
							if (table->desk[i] > 0) {
								far_cell = i;
								break;
							}
						}
						if (far_cell != -1) {
							table->aviable_moves[count_moves][0] = far_cell;
							table->aviable_moves[count_moves][1] = -1;
							table->aviable_moves[count_moves][2] = move_i;
							count_moves++;
						}
					}
				}
			}
			else if (table->cur_player == BLACK) {
				int cur_pos = 12 - table->dice[move_i];
				if (table->desk[cur_pos] < 0) {
					//выбрасываем
					table->aviable_moves[count_moves][0] = cur_pos;
					table->aviable_moves[count_moves][1] = -1;
					table->aviable_moves[count_moves][2] = move_i;
					count_moves++;
				}
				else {
					bool find_flag = false;
					for (int i = cur_pos; i >= 6; i--) {	//поиск хода внутри дома
						if (table->desk[i] < 0 && table->desk[i + table->dice[move_i]] <= 0 && (i + table->dice[move_i]) < 12) {
							table->aviable_moves[count_moves][0] = i;
							table->aviable_moves[count_moves][1] = i + table->dice[move_i];
							table->aviable_moves[count_moves][2] = move_i;
							count_moves++;
							find_flag = 1;
						}
					}
					if (!find_flag) {	//выброс с самого близкого занятого пункта
						int far_cell = -1;
						for (int i = 11; i >= 6; i--) {
							if (table->desk[i] < 0) {
								far_cell = i;
								break;
							}
						}
						if (far_cell != -1) {
							table->aviable_moves[count_moves][0] = far_cell;
							table->aviable_moves[count_moves][1] = -1;
							table->aviable_moves[count_moves][2] = move_i;
							count_moves++;
						}
					}
				}
			}
		}
	}
	return count_moves;
}

void make_move(table_s* table, int n) {
	if (table->aviable_moves[n - 1][1] == -1) {
		if (table->cur_player == WHITE) {
			table->desk[table->aviable_moves[n - 1][0]] -= 1;
			table->white_dropped++;
		}
		else if (table->cur_player == BLACK) {
			table->desk[table->aviable_moves[n - 1][0]] += 1;
			table->black_dropped++;
		}
		//table->desk[table->aviable_moves[n - 1][0]] -= table->cur_player == WHITE ? 1 : -1;
	}
	else {
		table->desk[table->aviable_moves[n - 1][0]] -= table->cur_player == WHITE ? 1 : -1;
		table->desk[table->aviable_moves[n - 1][1]] += table->cur_player == WHITE ? 1 : -1;
		if (table->dice[0] != table->dice[1])
			table->used_dice[table->aviable_moves[n - 1][2]] = true;
		if ((table->cur_player == WHITE && table->aviable_moves[n - 1][0] == 0) || (table->cur_player == BLACK && table->aviable_moves[n - 1][0] == 12)) {
			if (table->cur_player == WHITE && table->first_move_w && (table->dice[0] == table->dice[1] && (table->dice[0] == 3 || table->dice[0] == 4 || table->dice[0] == 6))) {
				table->first_move_w = false;
			}
			else if (table->cur_player == BLACK && table->first_move_b && (table->dice[0] == table->dice[1] && (table->dice[0] == 3 || table->dice[0] == 4 || table->dice[0] == 6))) {
				table->first_move_b = false;
			}
			else {
				table->head_used = true;
			}
		}
	}
}

void print_moves(int n) {
	if (n == 0)
		printf("Нет возможных ходов:(\n");
	else {
		printf("Возможные ходы:\n");
		for (int i = 0; i < n; i++) {
			printf("%d. Исходная позиция %d\tКонечная позиция %d\n", i + 1, table.aviable_moves[i][0], table.aviable_moves[i][1]);
		}
		printf("Введите номер желаемого хода: ");

		int cur_move;
		scanf("%d", &cur_move);
		make_move(&table, cur_move);
		print_desk();
	}
}

bool is_win() {
	if (all_home(table)) {
		if (table.cur_player == WHITE) {
			for (int i = 18; i < 24; i++) {
				if (table.desk[i] > 0) {
					return false;
				}
			}
		}
		else if (table.cur_player == BLACK) {
			for (int i = 6; i < 12; i++) {
				if (table.desk[i] < 0) {
					return false;
				}
			}
		}
	}
	else {
		return false;
	}
	return true;
}

int pip(table_s T, int player) {
	int sum = 0;
	for (int i = 0; i < 24; i++) {
		if (player == WHITE && T.desk[i] > 0) sum += T.desk[i] * (24 - i);
		if (player == BLACK && T.desk[i] < 0) sum += (-T.desk[i]) * ((11 - i + 24) % 24);
	}return sum;
}
int limit_for_depth(int deep, int unique) {
	int cap = (deep >= 2) ? 3 : (deep == 1) ? 5 : 7;
	return (unique > cap) ? cap : unique;
}
double evaluate(table_s T) {
	int player = T.cur_player;
	int enemy = -player;
	if (player == WHITE && T.black_dropped == 15) return -10000000;
	if (player == WHITE && T.white_dropped == 15) return 10000000;
	if (player == BLACK && T.white_dropped == 15) return -10000000;
	if (player == BLACK && T.black_dropped == 15) return 10000000;

	double sum = 2*(double)(pip(T, enemy) - pip(T, player));

	if (player == WHITE) {
		sum += T.white_dropped * 15;
		sum -= T.black_dropped * 15;
	}
	else {
		sum -= T.white_dropped * 15;
		sum += T.black_dropped * 15;
	}

	int lower_p = (player == 1) ? 18 : 6;
	int higher_p = (player == 1) ? 23 : 11;
	int lower_e = (player == -1) ? 18 : 6;
	int higher_e = (player == -1) ? 23 : 11;

	int run_p = 1; int run_e = 1;
	int prime_p = 1; int prime_e = 1;

	for (int i = 0; i < 24; i++) {
		int cur_p = (player == BLACK) ? (-T.desk[i]) : T.desk[i];
		int cur_e = (enemy == BLACK) ? (-T.desk[i]) : T.desk[i];

		if (cur_p && i >= lower_p && i <= higher_p) {
			sum += 1.5;
		}
		if (cur_e && i >= lower_e && i <= higher_e) {
			sum -= 1.5;
		}
		run_p = (cur_p > 0) ? run_p + 1 : 0;
		if (run_p > prime_p) prime_p = run_p;

		run_e = (cur_e > 0) ? run_e + 1 : 0;
		if (run_e > prime_e) prime_e = run_e;
	}
	sum += prime_p * 3;
	sum -= prime_e * 3;

	return sum;
}
double expect(table_s T, int deep);
double best_choose(table_s T, int deep, int dices) {
	int player = T.cur_player;
	int unique = 0;
	int size = all_moves(&T);
	double best = -10000000;
	if (size == 0 || dices <= 0) {
		if (deep > 0) {
			T.cur_player *= -1;
			return -expect(T, deep - 1);
		}
		return evaluate(T);
	}
	move Moves[60];
	for (int i = 0; i < size; i++) {
		
		bool done = false;
		for (int j = 0; j < i; j++) {
			if (T.aviable_moves[i][0] == T.aviable_moves[j][0] && T.aviable_moves[i][1] == T.aviable_moves[j][1]) {
				done = true; break;
			}
		}
		if (done) continue;

		table_s clone = T;
		Moves[unique].from = clone.aviable_moves[i][0];
		Moves[unique].to = clone.aviable_moves[i][1];
		Moves[unique].dice = clone.aviable_moves[i][2];
		make_move(&clone, i + 1);
		Moves[unique].score = evaluate(clone);
		unique++;
	}
	qsort(Moves, unique, sizeof(move), cmp);
	int limit = limit_for_depth(deep, unique);
	for (int i = 0; i < limit; i++) {
		double sum;
		table_s clone = T;

		clone.aviable_moves[0][0] = Moves[i].from;
		clone.aviable_moves[0][1] = Moves[i].to;
		clone.aviable_moves[0][2] = Moves[i].dice;

		make_move(&clone, 1);

		sum = best_choose(clone, deep, dices-1);
		if (sum > best) best = sum;
	}
	return best;
}
double best_for_roll(table_s T, int dice1, int dice2, int deep) {
	T.dice[0] = dice1; T.dice[1] = dice2;
	T.used_dice[0] = false; T.used_dice[1] = false;
	int dices = (dice1 == dice2) ? 4 : 2;
	return best_choose(T, deep, dices);
}

double expect(table_s T, int deep) {
	double total = 0, weight = 0;
	int d1[] = { 1,1,2,2,3,4,5,3 };
	int d2[] = { 2,6,3,5,4,5,6,4 };
	double Weight[8] = { 2.0 / 36.0, 2.0 / 36.0, 2.0 / 36.0 , 2.0 / 36.0 , 2.0 / 36.0 , 2.0 / 36.0 , 2.0 / 36.0 , 2.0 / 36.0 };
	
	for (int i = 0; i < 8; i++) {
		total += Weight[i] * best_for_roll(T, d1[i], d2[i], deep);
		weight += Weight[i];
	}
	return total / weight;
}

int bot_choose(int deep) {
	int size = all_moves(&table);
	if (size == 0) return 1;
	int dices = (table.dice[0] == table.dice[1]) ? 4 : 2;
	double best = -10000000; int best_i = 0, unique = 0; double sum;
	move Moves[60];
	for (int i = 0; i < size; i++) {

		bool done = false;
		for (int j = 0; j < i; j++) {
			if (table.aviable_moves[i][0] == table.aviable_moves[j][0] && table.aviable_moves[i][1] == table.aviable_moves[j][1]) {
				done = true; break;
			}
		}
		if (done) continue;

		table_s clone = table;
		Moves[unique].from = clone.aviable_moves[i][0];
		Moves[unique].to = clone.aviable_moves[i][1];
		Moves[unique].dice = clone.aviable_moves[i][2];
		Moves[unique].index = i;
		unique++;
	}
	for (int i = 0; i < unique; i++) {
		double sum;
		table_s clone = table;

		clone.aviable_moves[0][0] = Moves[i].from;
		clone.aviable_moves[0][1] = Moves[i].to;
		clone.aviable_moves[0][2] = Moves[i].dice;

		make_move(&clone, 1);

		sum = best_choose(clone, deep, dices - 1);
		if (sum > best) {
			best = sum;
			best_i = Moves[i].index;
		}
	}
	return best_i+1;
}

int tupoy_bot() {
	int size = all_moves(&table);
	return (rand() % size);
}
int main() {
	char* locale = setlocale(LC_ALL, "");
	srand(time(NULL));
	init_table();
	print_desk();
	char* cur_player_move;
	int move_cnt;
	int deep;
	scanf("%d", &deep);
	deep = deep % 3; 
	// сложность бота
	while (1) {
		reset_table();
		cur_player_move = table.cur_player == WHITE ? "Белый" : "Черный";
		printf("Ход игрока %s\n", cur_player_move);
		roll_dice();
		printf("Выпало: %d %d\n", table.dice[0], table.dice[1]);
		move_cnt = table.dice[0] == table.dice[1] ? 4 : 2;
		for (int i = 0; i < move_cnt; i++) {
			if (table.cur_player == BLACK && all_moves(&table)>0) {
				make_move(&table, bot_choose(deep));
			}
			else {
				int moves_count = all_moves(&table);

				print_moves(moves_count);
			}
		}

		if (table.cur_player == WHITE)
			table.first_move_w = false;
		if (table.cur_player == BLACK)
			table.first_move_b = false;

		if (is_win()) {		//ПОБЕДА
			printf("Поздравляем! Игрок %s выиграл!", cur_player_move);
			exit(0);
		}

		table.cur_player = table.cur_player == WHITE ? BLACK : WHITE;
		getchar();

	}


	return 0;
}