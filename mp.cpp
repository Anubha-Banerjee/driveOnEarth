#include <iostream>
#include <math3d.h>
#include "map.h"

using namespace std;

void Map::inputCoordinates()
{
  cout << "Enter the latitude : ";
	cin >> map_latitude;

	cout << "Enter the longitude : ";
	cin >> map_longitude;
}


void Map::calculateShift(double x, double y)
{
	int w = 2000;
	double latRad = m3dDegToRad(x + width/2);
	double lonRad = m3dDegToRad(y + length/2);
	double lonRad2 = m3dDegToRad(y);

	// apply the mercator projection to the top left corner of the bounding box, so we can shift the whole map on to the screen
	shiftX = -(((w / (2 * M_PI)) * (lonRad)  * SCALE)) ;
	shiftY = -((w / (2 * M_PI) * log(tan( M_PI / 4 +  latRad/ 2)) * SCALE) );
}


bool readWhiteSpaces(FILE *fp)
{	
	
	char ch;
	bool word_break = true;

	// eat ch if its newline
	while(true)
	{	
		fscanf(fp, "%c", &ch);
		if(ch == '\n')
		{	
			word_break = false;		
		}
		else
		{
			// go one back as loop will have to break as character not \n'
			fseek(fp, -1, SEEK_CUR);
			break;
		}
	}

	// eat ch if its space
	while(true)
	{	
		fscanf(fp, "%c", &ch);
		if(ch == ' ')
		{	
			word_break  = false;			
		}
		else
		{
			// go one back as loop will have to break as character not \n'
			fseek(fp, -1, SEEK_CUR);
			break;
		}
	}
	return word_break;
}

void readWord(FILE *fp, char word[50])
{
	char ch;
	char chunkSize[50], wordEnd[50];
	bool word_break = false;	
	
	fscanf(fp, "%s", word);

	// eat whitespaces
	word_break = readWhiteSpaces(fp);
	
	// read the next char
	fscanf(fp, "%c", &ch);
	if(ch == '\r')
	{
		fscanf(fp, "%s", chunkSize);
		cout << "\nchunk size : " << chunkSize;

		// read off \r and \n following the chunk size
		fscanf(fp, "%c", &ch);
		fscanf(fp, "%c", &ch);

		if(!strcmp(chunkSize, "0"))
		{
			cout << "end of input";
			strcpy(word, chunkSize);
			return;
		}

		// as if word break is false then it is seen that word is not broken by chunk size
		if(word_break == true)
			word_break = readWhiteSpaces(fp);

		if(word_break)
		{
			fscanf(fp, "%s", wordEnd);
			strcat(word, wordEnd);
			cout << "\nFull word: " << word;
		}
	}
	else
		fseek(fp, -1, SEEK_CUR);
	
}

double convert_lat_to_y(double lat)
{
		double y = 0, w = 2000;
		double latRad = m3dDegToRad(lat);
		y = (w / (2 * M_PI) * log(tan( M_PI / 4 + latRad / 2)) * SCALE) ;
		y =  y + map.shiftY;
		return y;
}

double convert_lon_to_x(double lon)
{
	double x = 0, w = 2000;
	float lonRad = m3dDegToRad(lon);
	//float ylonRad =  m3dDegToRad(90);

	x = ((w / (2 * M_PI)) * (lonRad)  * SCALE) ;
	x = x + map.shiftX;

	return x;
}

FILE *np = fopen("d:\\node.txt", "w+");
// this function stores the nodes from file fp and stores in array of nodes
void Map::store_nodes_in_array(FILE *fp)
{
	map.nodes = new node[MAX_NODES];

	int nodeIndex = 0;
	char word[1000];
	double lat, lon;
	
	fseek(fp, 0, SEEK_SET);
	while(!feof(fp))
	{
		readWord(fp, word);

		// break off if its last chunk size 0
		if(!strcmp(word, "0"))
			break;

		// break off if word is way
		if(!strcmp(word, "\"way\","))
		{
			fseek(fp, -10, SEEK_CUR);
			break;
		}

		if(!strcmp(word, "\"node\","))
		{
			// read "id":
			readWord(fp, word);
	
			// read the acutal node id value
			readWord(fp, word);		
						
			nodes[nodeIndex].id = atol(word);
			fprintf(np, "id:   %d  ", nodes[nodeIndex].id );
			
			// read "lat:"
			readWord(fp, word);		

			// read latitude value
			readWord(fp, word);		

			lat = atof(word);
			nodes[nodeIndex].y = convert_lat_to_y(lat);	
			fprintf(np, "laty:  %d", nodes[nodeIndex].y );
			
			// read "lon:"
			readWord(fp, word);		
			
			// read longitude value
			readWord(fp, word);		
			lon = atof(word);
			nodes[nodeIndex].x = convert_lon_to_x(lon);	
			fprintf(np, "lonx:   %d\n", nodes[nodeIndex].x );
			nodeIndex++;			
		}
	}
	cout << "\nno of nodes in llist" << nodeIndex;
	nodeCount = nodeIndex;
}

node *searchNode(long nodeID)
{
	bool found = 0;
	for(int i = 0; i < map.nodeCount; i++)
	{
		if(nodeID == map.nodes[i].id)
		{
			found = 1;
			return &map.nodes[i];
		}
	}

	if(!found)
	{
		cout << "\nnot found : " << nodeID ; 
		return 0;
	}

}

// this function stores the ways from nodes array
void Map::store_ways_in_array(FILE *fp)
{
	map.ways = new way[MAX_WAYS];

	int wayIndex = 0;
	char word[1000], ch = 'a';
		
	//fseek(fp, 0 , SEEK_SET);
	while(!feof(fp))
	{
		readWord(fp, word);	
		if(!strcmp(word, "0"))
			break;

		if(!strcmp(word, "\"way\",")) // encountered "way",
		{	
			// read "id:"
			readWord(fp, word);

			// read id value
			readWord(fp, word);

			fprintf(np, "\nwayId:    %s \n", word);

			//cout << "\n" << word << " " << wayIndex;
					
			ways[wayIndex].id = atol(word);

			// read "nodes":
			readWord(fp, word);

			readWord(fp, word); // read '['
			
			ways[wayIndex].nodeCount = 0;

			// loop reading off and storing the nodes in a way
			ch = '[';
			while(ch != ']' && !feof(fp))
			{
					// read node values
					readWord(fp, word);
										
					ways[wayIndex].nodeList[ways[wayIndex].nodeCount] = searchNode(atol(word));
									
					fprintf(np, "\nnode: %d", ways[wayIndex].nodeList[ways[wayIndex].nodeCount] -> id);
					fprintf(np, "   x:  %d", ways[wayIndex].nodeList[ways[wayIndex].nodeCount] -> x);
					fprintf(np, "   y:  %d", ways[wayIndex].nodeList[ways[wayIndex].nodeCount] -> y);

					ways[wayIndex].nodeCount++;

					readWhiteSpaces(fp);
					fscanf(fp, "%c", &ch); // check if its the ] following the last node value
					if(ch != ']')
						fseek(fp, -1, SEEK_CUR);
			}
		
			// one more way gone into ways array
			wayIndex++;
		}
	}
	map.wayCount = wayIndex;
	cout << "\n\nwayssssssssss : " << map.wayCount ;
}


void computeRectangleFromLineAndStore(float x1, float y1, float x2, float y2)
{
	float width = 0.8;
	double thetaSlope;

	float roadBase= x1 - x2;
	float roadPerp = y2 - y1;
	
	thetaSlope = atan(roadPerp / roadBase);
	
	float base = width * sin(thetaSlope);
	float perp = width * cos(thetaSlope);

	map.rects[map.rectCount].thetaSlope = thetaSlope;
	map.rects[map.rectCount].x1_right = x1 + base;
	map.rects[map.rectCount].y1_right = y1 + perp;

	map.rects[map.rectCount].x2_right = x2 + base;
	map.rects[map.rectCount].y2_right = y2 + perp;	

	map.rects[map.rectCount].x1_left = x1 - base;
	map.rects[map.rectCount].y1_left = y1 - perp;

	map.rects[map.rectCount].x2_left = x2 - base;
	map.rects[map.rectCount].y2_left = y2 - perp;	
}


void Map::store_rect_coords_in_array()
{
	int x, y;
	map.rects = new rectangle[MAX_NODES];

	for(int i = 0; i < map.wayCount; i ++)
	{
		float prev_x = INFINITY, prev_y = INFINITY;
		map.rects[map.rectCount-1].endRect = true;

		for(int j = 0; j < map.ways[i].nodeCount; j ++)
		{
			x = map.ways[i].nodeList[j]->x;
			y = map.ways[i].nodeList[j]->y;
	
			if(prev_x != INFINITY && prev_y != INFINITY)
			{					
				computeRectangleFromLineAndStore(prev_x, prev_y, x, y);
				map.rects[map.rectCount].endRect = false;
				map.rectCount++;
			}		
			prev_x = x;
			prev_y = y;
		}
	}
}


void fillRectArray(GLfloat rectArray[MAX_WAYS * 6][3], rectangle rectCurr, int *index)
{
	    int j = *index;
		// fill in rectArray
		// 0
		rectArray[j][0] = rectCurr.x2_left;
		rectArray[j][1] = 4;
		rectArray[j][2] = rectCurr.y2_left;

		j++;

		// 1
	
		rectArray[j][0] = rectCurr.x2_right;
		rectArray[j][1] = 4;
		rectArray[j][2] = rectCurr.y2_right;

		j++;

		// 2
		rectArray[j][0] = rectCurr.x1_left;
		rectArray[j][1] = 4;
		rectArray[j][2] = rectCurr.y1_left;

		j++;

		// 3
		rectArray[j][0] = rectCurr.x2_right;
		rectArray[j][1] = 4;
		rectArray[j][2] = rectCurr.y2_right;

		j++;

		// 4
		rectArray[j][0] = rectCurr.x1_left;
		rectArray[j][1] = 4;
		rectArray[j][2] = rectCurr.y1_left;

		j++;

		// 5
		rectArray[j][0] = rectCurr.x1_right;
		rectArray[j][1] = 4;
		rectArray[j][2] = rectCurr.y1_right;

		j++; 

		*index = j;
}

void fillJointArray(GLfloat jointArray[MAX_WAYS * 6][3], rectangle rectCurr, rectangle rectPrev, int *index)
{
	    int j = *index;

		// fill in rectArray
		// 0
		jointArray[j][0] = rectPrev.x2_left;
		jointArray[j][1] = 4;
		jointArray[j][2] = rectPrev.y2_left;
	
		j++;

		// 1	
		jointArray[j][0] = rectPrev.x2_right;
		jointArray[j][1] = 4;
		jointArray[j][2] = rectPrev.y2_right;

		j++;

		// 2
		jointArray[j][0] = rectCurr.x1_right;
		jointArray[j][1] = 4;
		jointArray[j][2] = rectCurr.y1_right;

		j++;

		// 3
		jointArray[j][0] = rectPrev.x2_left;
		jointArray[j][1] = 4;
		jointArray[j][2] = rectPrev.y2_left;

		j++;

		// 4
		jointArray[j][0] = rectPrev.x2_right;
		jointArray[j][1] = 4;
		jointArray[j][2] = rectPrev.y2_right;

		j++;

		// 5
		jointArray[j][0] = rectCurr.x1_left;
		jointArray[j][1] = 4;
		jointArray[j][2] = rectCurr.y1_left;

		j++; 

		*index = j;
}


void Map::store_rects_in_float_array()
{
	int j = 0;
	int k = 0;

	rectangle rectCurr, rectPrev;

	for(int i = 0; i < map.rectCount; i++)
	{
		rectCurr = map.rects[i];

		fillRectArray(map.rectArray, rectCurr, &j);
	
		// fill in jointArray if ending is false then only join the ways		
		if(map.rects[i - 1].endRect == false)
		{
			rectPrev = map.rects[i - 1];
			fillJointArray(map.jointArray, rectCurr, rectPrev, &k);
		}
	}	
}

