/*
	USING TUTORIAL FROM ONELONECODER (javidx9)
	FOR LEARNING PURPOSES
*/

#include <iostream>
#include <thread>
#include <vector>
#include <Windows.h>
#define UNICODE
#define _UNICODE
using namespace std;

wstring tetromino[7]; // use wstring over string for windows; holds more characters 

int nFieldWidth = 12; // playing field dimensions
int nFieldHeight = 18;

unsigned char* pField = nullptr; // playing field contents; allocated dynamically
								 // all map information is stored within the unsigned char array

int nScreenWidth = 120;	// Console screen size X
int nScreenHeight = 30; // Console screen size Y

int Rotate(int px, int py, int r){ // rotates pieces 

	/*
	Algorithm for rotating assets:
	(i = index of asset in playing field)
	0*: i = y * w + x
	90*: i = 12 + y - (x * 4)
	180*: i = 15 - (y * 4) - x
	270*: i = 3 - y + (x * 4)
	*/

	switch (r % 4) {
		case 0: 
			return py * 4 + px; // 0 degrees
		case 1: 
			return 12 + py - (px * 4); // 90 degrees
		case 2: 
			return 15 - (py * 4) - px; // 180 degrees
		case 3: 
			return 3 - py + (px * 4); // 270 degrees
	}
	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {

	for (int px = 0; px < 4; px++) 
		for (int py = 0; py < 4; py++)
		{
			// get index of piece
			int pi = Rotate(px, py, nRotation); 

			// get index of field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			// collision detection
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false;
				}

			}

		}
	
	

	return true; // always returns true
}

int main() {

	// create assets (blocks)
	tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	pField = new unsigned char[nFieldWidth * nFieldHeight]; // create play field buffer

	for (int x = 0; x < nFieldWidth; x++) // create board boundaries
		for (int y = 0; y < nFieldHeight; y++) 
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0; // value 9 represents border, 
																												//	0 equals empty space
																					
	

	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight]; // creates unsigned char array
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' '; // fills with blank space
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL); // creates handle to console buffer
	SetConsoleActiveScreenBuffer(hConsole); // sets it as active buffer
	DWORD dwBytesWritten = 0;

	// game loop 
	
	
	bool bGameOver = false;

	int nCurrentPiece = 0; 
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2; // middle of screen
	int nCurrentY = 0; // top of screen

	bool bKey[4];
	bool bRotateHold = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;
	int nScore = 0;

	vector<int> vLines;

	while (!bGameOver) {

		// GAME TIMING ================

		this_thread::sleep_for(50ms); // one game tick
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);
		

		// INPUT ==================
		for (int k = 0; k < 4; k++)								// R   L   D  Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k])));
	


		// GAME LOGIC =========================


		if (bKey[1]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) {
				nCurrentX = nCurrentX - 1;
			}
		}

		if (bKey[0]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) {
				nCurrentX = nCurrentX + 1;
			}
		}

		if (bKey[2]) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY = nCurrentY + 1;
			}
		}
		if (bKey[3]) {
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true; // prevents crazy rotation 
		}
		else {
			bRotateHold = false;
		}

		if (bForceDown) {
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
				nCurrentY++; // fit the piece if it can do it
			}
			else {
				// lock current piece in place
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				nPieceCount++;
				if (nPieceCount % 10 == 0) {
					if (nSpeed >= 10) {
						nSpeed--;
					} 
				}
					

				// check have we got a line
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1) {

						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++) 
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0; 

						// remove line, set to =
						if (bLine) {
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;

							vLines.push_back(nCurrentY + py);
						}						
					}

				// score handling 
				nScore += 25;
				if (!vLines.empty()) {
					nScore += (1 << vLines.size()) * 100; // add 250 pts if player gets a line
				} 
				 
				
				// choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// if piece does not fit, game over
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCounter = 0;
		}

		// RENDER OUTPUT ============================


		// draw field
		for (int x = 0; x < nFieldWidth; x++) 
			for (int y = 0; y < nFieldHeight; y++) 
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]]; // string contains screen elements (0 is blank, 1-7 are assets, 8 is line, 9 is boundary) // screen is offset by two to avoid drawing to corner of console
			
		// draw current piece 
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.') // iterate through piece, if it's equal to an X, draw to screen
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65; // 65 is A in ASCII

		// draw score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		// manage lines
		if (!vLines.empty()) {

			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);


			for (auto& v : vLines) 
				for (int px = 1; px < nFieldWidth - 1; px++) {
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}
			vLines.clear();
		}


		// display output
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten); 

	}

	// game over
	CloseHandle(hConsole);
	cout << "Game Over! Score: " << nScore << endl;
	system("pause");

	
	return 0;
}