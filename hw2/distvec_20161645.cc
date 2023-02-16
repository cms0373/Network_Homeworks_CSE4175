#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------- global variables ------------

#define COST_INF 9999

typedef struct ROUTING_TABLE {
	int dest;
	int next;
	int cost;
} ROUTING_TABLE;

typedef struct MESSAGE {
	int from;
	int to;
	char msg[1000];
} MESSAGE;

typedef struct CHANGE {
	int node1;
	int node2;
	int cost;
} CHANGE;

int node_num = 0;
ROUTING_TABLE routing_tab[100][100];

int msg_num = 0;
MESSAGE msg_list[100];

int change_num = 0;
CHANGE change_list[100];

int topology[100][100];

// -------------------------------------

void save_messagesfile(FILE* fp) {
	char message[1000];
	int cnt = 0;

	while (fgets(message, sizeof(message), fp) != NULL) {
		msg_list[cnt].from = message[0] - 48;
		msg_list[cnt].to = message[2] - 48;
		strcpy(msg_list[cnt].msg, message + 4);
		cnt++;
	}
	msg_num = cnt;
}

void save_changesfile(FILE* fp) {
	int node1, node2, cost;
	while (fscanf(fp, "%d %d %d", &node1, &node2, &cost) != EOF) {
		change_list[change_num].node1 = node1;
		change_list[change_num].node2 = node2;
		change_list[change_num].cost = cost;
		change_num++;
	}
	return;
}

void print_routing_tab(FILE* fp) {
	int i, j;

	for (i = 0; i < node_num; i++) {
		for (j = 0; j < node_num; j++) {
			if (routing_tab[i][j].cost != COST_INF)
				fprintf(fp, "%d %d %d\n", routing_tab[i][j].dest, routing_tab[i][j].next, routing_tab[i][j].cost);
				//printf("%d %d %d\n", routing_tab[i][j].dest, routing_tab[i][j].next, routing_tab[i][j].cost);
		}
		fprintf(fp, "\n");
		//printf("\n");
	}
	return;

}

void print_message(FILE* fp) {
	int i;
	int from, to, cost;
	int hop;

	for (i = 0; i < msg_num; i++) {
		from = msg_list[i].from;
	   	to = msg_list[i].to;
	   	cost = routing_tab[from][to].cost;
        if (cost < COST_INF) {
            fprintf(fp, "from %d to %d cost %d ", from, to, cost);
		    fprintf(fp, "hops %d ", from);
		    hop = from;
		    while (1) {
			    if (routing_tab[hop][to].next == to)
				    break;
			    fprintf(fp, "%d ", routing_tab[hop][to].next);
			    hop = routing_tab[hop][to].next;
		    }
		    fprintf(fp, "%s", msg_list[i].msg);
        }
        else
            fprintf(fp, "from %d to %d cost infinite hops unreachable message %s", from, to, msg_list[i].msg);
    }
	fprintf(fp, "\n");
	return;
}

void init_routing_tab() {
	int i, j;

	for (i = 0; i < node_num; i++) {
		for (j = 0; j < node_num; j++) {
			routing_tab[i][j].dest = j;
			routing_tab[i][j].cost = topology[i][j];
			
			if (topology[i][j] >= 0 && topology[i][j] != COST_INF)
				routing_tab[i][j].next = j;
			else
				routing_tab[i][j].next = -1;
		}
	}
}

int refresh_routing_tab() {
	int i, j, k;
	int changed = 0;

	for (i = 0; i < node_num; i++) {
		for (j = 0; j < node_num; j++) { // j send its routing table to i and change i's routing table
			if (topology[i][j] > 0 && topology[i][j] != COST_INF) { // if they're neighbors
				for (k = 0; k < node_num; k++) { // read neighbor's routing table 
					if (routing_tab[i][k].cost > routing_tab[j][k].cost + topology[i][j]) {
						routing_tab[i][k].cost = routing_tab[j][k].cost + topology[i][j];
						routing_tab[i][k].next = j;
						changed = 1;
					}
					// tie-breaking rule 1
					else if (routing_tab[i][k].cost == routing_tab[j][k].cost + topology[i][j]) {
						if (j < routing_tab[i][k].next) {
							routing_tab[i][k].next = j;
							changed = 1;
						}
					}
				}
			}
		}
	}

	return changed;
}

int main(int argc, char* argv[]) {
	FILE* fp1, * fp2, * fp3, * fp_out;
	int node1, node2, cost;
	int converge;

	if (argc != 4) {
		printf("usage: distvec topologyfile messagesfile changesfile\n");
		return 1;
	}

	fp1 = fopen(argv[1], "r");
	fp2 = fopen(argv[2], "r");
	fp3 = fopen(argv[3], "r");
	fp_out = fopen("output_dv.txt", "w");

	if (fp1 == NULL) {
		printf("Error: open input file. topology");
		return 1;
	}
	if (fp2 == NULL) {
		printf("Error: open input file. message");
		return 1;
	}
	if (fp3 == NULL) {
		printf("Error: open input file. change");
		return 1;
	}

	// handle topology file
	fscanf(fp1, "%d", &node_num);
	for (int i = 0; i < node_num; i++) {
		for (int j = 0; j < node_num; j++) {
			topology[i][j] = COST_INF;
		}
	}

	while (fscanf(fp1, "%d %d %d", &node1, &node2, &cost) != EOF) {
		topology[node1][node2] = cost;
		topology[node2][node1] = cost;
	}
	for (int i = 0; i < node_num; i++)
		topology[i][i] = 0;

	// handle messages, changes file
	save_messagesfile(fp2);
	save_changesfile(fp3);

	// initialize routing table
	init_routing_tab();

	// refresh routing table
	converge = 0;
	while (converge != 20) {
		if (refresh_routing_tab() == 0)
			converge++;
	}

	// print routing table and message
	print_routing_tab(fp_out);
	print_message(fp_out);

	
	for (int i = 0; i < change_num; i++) {
		node1 = change_list[i].node1;
		node2 = change_list[i].node2;
		cost = change_list[i].cost;
		
		if (cost == -999) { // delete link
			topology[node1][node2] = COST_INF;
			topology[node2][node1] = COST_INF;
		}
		else { // change cost
			topology[node1][node2] = cost;
			topology[node2][node1] = cost;
		}

		// initialize routing table
		init_routing_tab();

		// refresh routing table
		converge = 0;
		while (converge != 20) {
			if (refresh_routing_tab() == 0)
				converge++;
		}

		// print routing table and message
		print_routing_tab(fp_out);
		print_message(fp_out);

	}

	printf("Complete. Output file written to output_dv.txt.\n");

	fclose(fp1); fclose(fp2); fclose(fp3); fclose(fp_out);

	return 0;
}
