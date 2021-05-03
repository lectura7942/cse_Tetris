#include "tetris.h"

static struct sigaction act, oact;

int main() {
	int exit = 0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);
	createRankList(); // 랭킹 자료구조 구축
	srand((unsigned int)time(NULL));

	recRoot = (RecNode*)malloc(sizeof(RecNode)); 
	recRoot->lv = -1; 

	while (!exit) {
		clear();
		switch (menu()) {
		case MENU_PLAY: playVersion = MENU_PLAY;  play(); break;
		case MENU_RANK: rank(); break;
		case MENU_REC_PLAY: playVersion = MENU_REC_PLAY;  recommendedPlay(); break;
		case MENU_EXIT: exit = 1; break;
		default: break;
		}
	}

	free(recRoot);
	endwin();
	echo();
	writeRankFile();
	system("clear");
	return 0;
}

void InitTetris() {
	int i, j;

	for (j = 0; j < HEIGHT; j++)
		for (i = 0; i < WIDTH; i++)
			field[j][i] = 0;

	for (i = 0; i < BLOCK_NUM; i++) nextBlock[i] = rand() % 7;
	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;
	score = 0;
	gameOver = 0;
	timed_out = 0;

	DrawOutline();
	DrawField();
	modified_recommend(recRoot);
	if(playVersion==MENU_PLAY) DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate); 
	else {
		// remove shadow for rec_play
		DrawBlock(recommendY, recommendX, nextBlock[0], recommendR, 'R'); // 현재 블록의 추천 위치 chk
		DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' '); // 현재 블록
	}
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline() {
	int i, j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0, 0, HEIGHT, WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2, WIDTH + 10);
	printw("NEXT BLOCK");
	DrawBox(3, WIDTH + 10, 4, 8);

	/*두 번째 next block을 보여주는 공간의 테두리*/
	DrawBox(9, WIDTH + 10, 4, 8);

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(15, WIDTH + 10); //
	printw("SCORE");
	DrawBox(16, WIDTH + 10, 1, 8);
}

int GetCommand() {
	int command;
	command = wgetch(stdscr);
	switch (command) {
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command) {
	int ret = 1;
	int drawFlag = 0;
	switch (command) {
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if ((drawFlag = CheckToMove(field, nextBlock[0], (blockRotate + 1) % 4, blockY, blockX)))
			blockRotate = (blockRotate + 1) % 4;
		break;
	case KEY_DOWN:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX + 1)))
			blockX++;
		break;
	case KEY_LEFT:
		if ((drawFlag = CheckToMove(field, nextBlock[0], blockRotate, blockY, blockX - 1)))
			blockX--;
		break;
	default:
		break;
	}
	if (drawFlag) DrawChange(field, command, nextBlock[0], blockRotate, blockY, blockX);
	return ret;
}

void DrawField() {
	int i, j;
	for (j = 0; j < HEIGHT; j++) {
		move(j + 1, 1);
		for (i = 0; i < WIDTH; i++) {
			if (field[j][i] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score) {
	move(17, WIDTH + 11);
	printw("%8d", score);
}

void DrawNextBlock(int* nextBlock) {
	int i, j;
	for (i = 0; i < 4; i++) {
		move(4 + i, WIDTH + 13);
		for (j = 0; j < 4; j++) {
			if (block[nextBlock[1]][0][i][j] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
	for (i = 0; i < 4; i++) {
		move(10 + i, WIDTH + 13);
		for (j = 0; j < 4; j++) {
			if (block[nextBlock[2]][0][i][j] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID, int blockRotate, char tile) {
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			if (block[blockID][blockRotate][i][j] == 1 && i + y >= 0) {
				move(i + y + 1, j + x + 1);
				attron(A_REVERSE);
				printw("%c", tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT, WIDTH + 10);
}

void DrawBox(int y, int x, int height, int width) {
	int i, j;
	move(y, x);
	addch(ACS_ULCORNER);
	for (i = 0; i < width; i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for (j = 0; j < height; j++) {
		move(y + j + 1, x);
		addch(ACS_VLINE);
		move(y + j + 1, x + width + 1);
		addch(ACS_VLINE);
	}
	move(y + j + 1, x);
	addch(ACS_LLCORNER);
	for (i = 0; i < width; i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play() {
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	do {
		if (timed_out == 0) {
			alarm(1);
			timed_out = 1;
		}

		command = GetCommand();
		if (ProcessCommand(command) == QUIT) {
			alarm(0);
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			refresh();
			getch();
			newRank(score);
			return;
		}
	} while (!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu() {
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////

int CheckToMove(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX) {
	// user code
	int fy, fx;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j] == 1) {
				fy = i + blockY;
				fx = j + blockX; // 필드 상 위치
				if (!(0 <= fy && fy < HEIGHT && 0 <= fx && fx < WIDTH)) return 0; // 필드에서 벗어남
				if (f[fy][fx] == 1) return 0; // 필드 위에 이미 블록이 있음
			}

		}
	}
	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH], int command, int currentBlock, int blockRotate, int blockY, int blockX) {
	// user code
	int preRotate = blockRotate, preY = blockY, preX = blockX;
	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	switch (command) {
	case KEY_UP:
		preRotate = (blockRotate + 3) % 4;
		break;
	case KEY_DOWN:
		preY--;
		break;
	case KEY_RIGHT:
		preX--;
		break;
	case KEY_LEFT:
		preX++;
		break;
	default:
		break;
	}
	//2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	int fy, fx;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			fy = i + preY;
			fx = j + preX;
			if (block[currentBlock][preRotate][i][j] == 1 && 0 <= fy && fy < HEIGHT && 0 <= fx && fx < WIDTH) {
				f[fy][fx] = 0;
			}
		}
	}
	DrawField();
	//3. 새로운 블록 정보를 그린다. 
	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate); //
	move(HEIGHT, WIDTH + 10); // 커서 필드 밖으로 이동
}

void BlockDown(int sig) {
	// user code
	//강의자료 p26-27의 플로우차트를 참고한다.
	if (CheckToMove(field, nextBlock[0], blockRotate, blockY + 1, blockX)) {
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	}
	else {
		if (blockY == -1) {
			gameOver = 1;
		}
		score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
		score += DeleteLine(field);
		PrintScore(score);
		for (int i = 0; i < BLOCK_NUM-1; i++) {
			nextBlock[i] = nextBlock[i + 1];
		}
		nextBlock[BLOCK_NUM - 1] = rand() % 7;
		//nextBlock[0] = nextBlock[1];
		//nextBlock[1] = nextBlock[2];
		//nextBlock[2] = rand() % 7; 
		DrawNextBlock(nextBlock);
		modified_recommend(recRoot); // initiallized in main()
		//Initiallize location of currentBlock
		blockRotate = 0;
		blockY = -1;
		blockX = WIDTH / 2 - 2;
		DrawField();
	}
	timed_out = 0;
}

int AddBlockToField(char f[HEIGHT][WIDTH], int currentBlock, int blockRotate, int blockY, int blockX) {
	// user code
	//Block이 추가된 영역의 필드값을 바꾼다.
	touched_field = 0; touched_side = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j] == 1){
				if (blockY + i == HEIGHT - 1) touched_field++;
				else if(f[blockY + i + 1][blockX + j] == 1) touched_field++;
				if (blockX + j == 0 || block + j == WIDTH - 1) touched_side++;
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (block[currentBlock][blockRotate][i][j] == 1) {
				f[blockY + i][blockX + j] = 1;
			}
		}
	}
	return touched_field * 10;
}

int DeleteLine(char f[HEIGHT][WIDTH]) {
	// user code

	//1. 필드를 탐색하여, 꽉 찬 구간이 있는지 탐색한다.
	//2. 꽉 찬 구간이 있으면 해당 구간을 지운다. 즉, 해당 구간으로 필드값을 한칸씩 내린다.
	int flag; // 1: 꽉 찬 구간
	full_line = 0; // 꽉 찬 구간 개수
	for (int i = 0; i < HEIGHT; i++) {
		flag = 1;
		for (int j = 0; j < WIDTH; j++) {
			if (f[i][j] == 0) {
				flag = 0; break;
			}
		}
		if (flag) {
			full_line++;
			for (int k = i; k > 0; k--) { // 해당 구간으로 필드값 한칸씩 내리기
				for (int m = 0; m < WIDTH; m++) {
					f[k][m] = f[k - 1][m];
				}
			}
			// 맨 위 가로줄 비우기
			for (int m = 0; m < WIDTH; m++) f[0][m] = 0;
		}
	}
	return full_line * full_line * 100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID, int blockRotate) {
	// user code
	char tile = '/';
	while (CheckToMove(field, blockID, blockRotate, y + 1, x)) y++;
	DrawBlock(y, x, blockID, blockRotate, tile);
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate) {
	DrawRecommend(recommendY, recommendX, blockID, recommendR);
	DrawShadow(y, x, blockID, blockRotate);
	DrawBlock(y, x, blockID, blockRotate, ' ');
}

void createRankList() {
	FILE* fp = fopen("rank.txt", "r");
	if (fp == NULL) {
		endwin();
		fprintf(stderr, "Cannot open rank.txt\n");
		exit(1);
	}
	char name_input[NAMELEN+1];
	int score_input;
	int cnt = 0;
	if (fscanf(fp, "%d%*c", &ranknum) != EOF) {
		while (fscanf(fp, "%s%d%*c", name_input, &score_input) != EOF) {
			node* new = (node*)malloc(sizeof(node));
			if (new == NULL) {
				endwin();
				fprintf(stderr, "Memmory allocation failed\n");
				exit(1);
			}

			cnt++;
			new->score = score_input;
			strcpy(new->name, name_input);
			if (rankptr) {
				if (score_input > rankptr->score) {
					new->link = rankptr;
					rankptr = new;
				}
				else {
					node* pre = rankptr;
					while (pre->link != NULL && pre->link->score >= score_input) pre = pre->link;
					new->link = pre->link;
					pre->link = new;
				}
			}
			else {
				new->link = rankptr;
				rankptr = new;
			}

		}
		ranknum = cnt;
	}
	fclose(fp);
}

void rank() {
	int X = 1, Y=ranknum, ch;

	clear();
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");
	ch = wgetch(stdscr);

	echo();
	if (ch == '1') {
		printw("X: ");
		scanw("%d", &X); 
		printw("Y: ");
		scanw("%d", &Y);

		printw("         name         |       score          \n");
		printw("---------------------------------------------\n");
		if (X <= 0 || Y > ranknum || X>Y) {
			printw("search failure: no rank in the list\n");
		}
		else {
			int count = 1;
			node* cur = rankptr;
			while (count < X) {
				cur = cur->link;
				count++;
			}
			while (count <= Y) {
				printw(" %-20s | %-20d\n", cur->name, cur->score);
				count++;
				cur = cur->link;
			}
		}
	}
	else if (ch == '2') {
		// 과제: 사용자 탐색
		char name_input[NAMELEN + 1];
		int found = 0;
		printw("Input the name: ");
		scanw("%s", &name_input);

		printw("         name         |       score          \n");
		printw("---------------------------------------------\n");
		node* cur = rankptr;
		while (cur != NULL) {
			if (strcmp(cur->name, name_input) == 0) {
				printw(" %-20s | %-20d\n", cur->name, cur->score);
				found = 1;
			}
			cur = cur->link;
		}
		
		if(found==0) printw("search failure: no name in the list\n");

	}
	else if (ch == '3') {
		// 과제: 삭제
		int rank_input;
		printw("Input the rank: ");
		scanw("%d", &rank_input);

		if (1 <= rank_input && rank_input <= ranknum) {
			node* cur = rankptr;
			if (rank_input == 1) {
				rankptr = cur->link;
			}
			else {
				node* pre = NULL;
				int count = 1; // cur가 가리키는 랭킹
				while (count < rank_input) {
					pre = cur;
					cur = cur->link;
					count++;
				}

				pre->link = cur->link;
			}
			free(cur);
			ranknum--;
			printw("\nresult: the rank deleted\n");
		}
		else printw("\nsearch failure: the rank not in the list\n");
	}
	noecho();
	getch(); // 스크린 유지
}

void writeRankFile() {

	FILE* fp = fopen("rank.txt", "w");
	if (fp == NULL) {
		endwin();
		fprintf(stderr, "Cannot open rank.txt\n");
		exit(1);
	}
	fprintf(fp, "%d\n", ranknum);
	node* del = rankptr;
	while (del != NULL) {
		rankptr = del->link;
		fprintf(fp, "%s %d\n", del->name, del->score);
		free(del);
		del = rankptr;
	}

	fclose(fp);
}

void newRank(int score) {
	char newname[NAMELEN+1];
	clear();
	echo();
	printw("your name: ");
	scanw("%s", newname);
	node* new = (node*)malloc(sizeof(node));
	if (new) {
		ranknum++;
		new->score = score;
		strcpy(new->name, newname);
		if (rankptr) {
			if (score > rankptr->score) {
				new->link = rankptr;
				rankptr = new;
			}
			else {
				node* pre = rankptr;
				while (pre->link != NULL && pre->link->score >= score) pre = pre->link;
				new->link = pre->link;
				pre->link = new;
			}
		}
		else {
			new->link = rankptr;
			rankptr = new;
		}
	}
	else {
		endwin();
		fprintf(stderr, "Memory allocation fail\n");
		exit(1);
	}
	noecho();

}

void DrawRecommend(int y, int x, int blockID, int blockRotate) {
	// user code
	DrawBlock(y, x, blockID, blockRotate, 'R');
}

int recommend(RecNode* root) {
	// 현재 레벨은 nextblock 인덱스를 따른다.
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	
	// user code
	if(root->lv==-1){
		memcpy(root->f, field, sizeof(field));
	}

	int childX, childY, childR;
	int idx = -1;
	int childID = nextBlock[root->lv + 1];

	for (childR = 0; childR < NUM_OF_ROTATE; childR++) {
		for (childX = 0; childX < WIDTH; childX++) {
			childY = -1; 
			while (CheckToMove(root->f, childID, childR, childY + 1, childX)) childY++;
			if (childY == -1) continue;

			idx++;
			root->c[idx] = (RecNode*)malloc(sizeof(RecNode));
			root->c[idx]->lv = root->lv + 1;

			memcpy(root->c[idx]->f, root->f, sizeof(field)); // 필드 복사
			root->c[idx]->score = AddBlockToField(root->c[idx]->f, childID, childR, childY, childX);
			root->c[idx]->score += DeleteLine(root->c[idx]->f);

			if (root->c[idx]->lv < BLOCK_NUM - 1) {
				root->c[idx]->score += recommend(root->c[idx]);
			}

			if (root->c[idx]->score >= max) {
				if (root->lv == -1) {
					recommendY = childY;
					recommendX = childX;
					recommendR = childR;	
				}
				max = root->c[idx]->score;
			}
		}
	}

	for (int i = 0; i <= idx; i++) free(root->c[i]);
	return max;
}

void recommendedPlay() { 
	// user code
	int command;
	clear();
	act.sa_handler = RecDown; 
	sigaction(SIGALRM, &act, &oact);
	InitTetris();
	do {
		if (timed_out == 0) {
			alarm(1);
			timed_out = 1;
		}

		if (GetCommand()==QUIT) {
			alarm(0);
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			refresh();
			getch();
			//newRank(score);  //chk
			return;
		}
	} while (!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	refresh();
	getch();
	//newRank(score);  // chck
}

int modified_recommend(RecNode* root) {
	// 현재 레벨은 nextblock 인덱스를 따른다.
	int res = -2000000000;// 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수

	// user code
	if (root->lv == -1) {
		recommendX = recommendY = recommendR = 0; // chk
		memcpy(root->f, field, sizeof(field));
	}

	int childX, childY, childR;
	RecNode* child = (RecNode*)malloc(sizeof(RecNode));
	child->lv = root->lv + 1;
	int childID = nextBlock[child->lv];

	int rotateNum;
	switch (childID) {
	case 4: rotateNum = 1; break;
	case 1:
	case 2:
	case 3: rotateNum = 4; break;
	default: rotateNum = 2;
	}

	for (childR = 0; childR < rotateNum; childR++) {
		for (childX = 0; childX < WIDTH; childX++) {
			childY = -1;
			while (CheckToMove(root->f, childID, childR, childY + 1, childX)) childY++;
			if (childY == -1) continue;

			memcpy(child->f, root->f, sizeof(field)); // 필드 복사
			child->score = AddBlockToField(child->f, childID, childR, childY, childX);
			child->score += DeleteLine(child->f);
			child->score += touched_field * WFIELD + touched_side * WSIDE + full_line * WLINE; /* 가중치 chk */

			if (child->lv < BLOCK_NUM - 1) {
				child->score += modified_recommend(child);
			}

			/* 가중치 chk*/
			child->score += FindValue(child->f);

			if (child->score >= res) {
				if (root->lv == -1) {
					recommendY = childY;
					recommendX = childX;
					recommendR = childR;
				}
				res = child->score;
			}
		}
	}

	free(child);
	return res;
}

void RecDown(int sig) {
	// user code

	// chk
	move(HEIGHT, WIDTH + 10);
	printw("(%d, %d) %d", recommendY, recommendX, recommendR);
	//

	if (!CheckToMove(field, nextBlock[0], blockRotate, blockY+1,blockX)) {
		gameOver = 1;
		return;
	}
	score += AddBlockToField(field, nextBlock[0], recommendR, recommendY, recommendX);
	score += DeleteLine(field);
	PrintScore(score);
	for (int i = 0; i < BLOCK_NUM - 1; i++) {
		nextBlock[i] = nextBlock[i + 1];
	}
	nextBlock[BLOCK_NUM - 1] = rand() % 7;
	DrawNextBlock(nextBlock);
	modified_recommend(recRoot); // initiallized in main() --> modified
	//Initiallize location of currentBlock
	blockRotate = 0;
	blockY = -1;
	blockX = WIDTH / 2 - 2;
	DrawField();
	DrawBlock(recommendY, recommendX, nextBlock[0], recommendR, 'R'); // 현재 블록의 추천 위치 chk
	DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' '); // 현재 블록
	timed_out = 0;
}

int FindValue (char f[HEIGHT][WIDTH]) { // chk

	int hole = 0, cover = 0, total_height = 0, max_height=0; 
	// hole, cover, total_height, max_height: 작을수록 좋음.  
	//side: 클수록 좋음
	int tc, start;
	for (int i = 0; i < WIDTH; i++) {
		start = 0; tc = 0;
		for (int j = 0; j < HEIGHT; j++) {
			if (f[j][i] == 1) {
				if (start) {
					tc++;
				}
				else {
					start = 1;
					total_height += (22-j);
					if (j > max_height) max_height = (22-j);
					tc++;
				}
			}
			else {
				if (start) {
					hole++;
					cover += tc; tc = 0;
				}
			}
		}
	}

	int res = hole * WHOLE + cover * WCOVER + total_height * WTOTAL_HEIGHT + max_height * WMAX_HEIGHT;
	return res;
}
