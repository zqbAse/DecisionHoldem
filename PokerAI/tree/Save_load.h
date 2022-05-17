################################################################################
#
#   Copyright 2022 The DecisionHoldem Authors，namely，Qibin Zhou，
#   Dongdong Bai，Junge Zhang and Kaiqi Huang. All Rights Reserved.
#
#   Licensed under the GNU AFFERO GENERAL PUBLIC LICENSE
#                 Version 3, 19 November 2007
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU Affero General Public License for more details.
#
#   You should have received a copy of the GNU Affero General Public License 
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
################################################################################
#pragma once
#include "Node.h"
#include "Bulid_Tree.h"
ofstream fout;
extern vector<strategy_node*> chancenodes;
void dfs_write(strategy_node* privatenode[], int clusterlen) {		//save strategy
	int len = privatenode[0]->action_len;
	fout.write((char*)&len, sizeof(int));
	if (len == 0)
		return;
	else if (len < 100 && len > 0) {
		for (int j = 0; j < len; j++)
			fout.write((const char*)&privatenode[0]->actions[j].actionstr, sizeof(unsigned char));
		for (int i = 0; i < clusterlen; i++) {
			assert(privatenode[i]->action_len == len);
			for (int j = 0; j < len; j++) {
				int te[] = { privatenode[i]->actions[j].regret ,privatenode[i]->actions[j].averegret };
				fout.write((const char*)te, sizeof(te));
			}
			//	fout.write((const char*)privatenode[i]->averegret, sizeof(int) * len);
		}
		strategy_node** tempprivatenode = new strategy_node * [clusterlen];
		for (int i = 0; i < len; i++) {
			for (int j = 0; j < clusterlen; j++)
				tempprivatenode[j] = privatenode[j]->actions + i;
			dfs_write(tempprivatenode, clusterlen);
		}
		delete[] tempprivatenode;
	}
	else if (len > 100) {
		strategy_node** privatenode2 = new strategy_node * [len];
		for (int j = 0; j < len; j++)
			privatenode2[j] = privatenode[0]->actions + j;
		dfs_write(privatenode2, len);
		delete[] privatenode2;
		//fout.write((const char*)cnode->regret, sizeof(int) * len);
	}
	else throw exception();
}
void dump(strategy_node* root, char filename[]) {
	fout.open(filename, ios::out | ios::binary);
	strategy_node* privatenode[169];
	for (int i = 0; i < 169; i++)
		privatenode[i] = root->actions + i;
	dfs_write(privatenode, 169);
	fout.close();
}

ifstream fin;
int countterminal = 0;
void bulid_bluestrategy(strategy_node* privatenode[], int clusterlen) {
	int len;
	fin.read((char*)&len, sizeof(int));
	if (len <= 0) {
		for (int i = 0; i < clusterlen; i++)
			privatenode[i]->action_len = 0;
		countterminal++;
		return; 
	}
	else if (len < 100) {
		unsigned char actionstr[12];// (char*)malloc(sizeof(char) * cnode->action_len);
		fin.read((char*)actionstr, len);
		for (int i = 0; i < clusterlen; i++) {
			privatenode[i]->init_child(actionstr, len);
			assert(len == privatenode[i]->action_len);
			for (int j = 0; j < len; j++) {
				int te[2];
				fin.read((char*)te, sizeof(te));
				privatenode[i]->actions[j].regret = te[0];
				privatenode[i]->actions[j].averegret = te[1];
			}
		}
		strategy_node** tempprivatenode = new strategy_node * [clusterlen];
		for (int i = 0; i < len; i++) {
			for (int j = 0; j < clusterlen; j++)
				tempprivatenode[j] = privatenode[j]->actions + i;
			bulid_bluestrategy(tempprivatenode, clusterlen);
		}
		delete[] tempprivatenode;
	}
	else if (len >= 100) {
		privatenode[0]->init_chance_node(len);
		chancenodes.push_back(privatenode[0]);
		for (int j = 1; j < clusterlen; j++) {
			privatenode[j]->action_len = len;
			privatenode[j]->actions = privatenode[0]->actions;
		}
		strategy_node** privatenode2 = new strategy_node * [len];
		for (int j = 0; j < len; j++)
			privatenode2[j] = privatenode[0]->actions + j;
		bulid_bluestrategy(privatenode2, len);
		delete[] privatenode2;
	}
	else throw exception();
}
void load(strategy_node* root, char filename[]) {//316174
	fin.open(filename, ios::in | ios::binary);
	root->init_chance_node(169);
	chancenodes.push_back(root);
	strategy_node* privatenode[169];
	for (int i = 0; i < 169; i++)
		privatenode[i] = root->actions + i;
	countterminal = 0;
	bulid_bluestrategy(privatenode, 169);
	cout << "countterminal:" << countterminal << endl;
	fin.close();
}
void initialstrategy(strategy_node* privatenode[], int clusterlen, bool exist) {
	int len;
	if (fin.eof())
		return;
	fin.read((char*)&len, sizeof(int));
	if (len <= 0) {
		return;
	}
	else if (len < 100) {
		unsigned char actionstr[12];
		fin.read((char*)actionstr, len);
		int oldaction_map_newaction[12];

		for (int i = 0; i < len; i++) {
			oldaction_map_newaction[i] = -1;
			if (exist)
				for (int j = 0; j < privatenode[0]->action_len; j++) {
					if (privatenode[0]->actions[j].actionstr == actionstr[i]) {
						oldaction_map_newaction[i] = j;
						break;
					}
				}
		}
		for (int i = 0; i < clusterlen; i++) {
			for (int j = 0; j < len; j++) {
				int te[2];
				fin.read((char*)te, sizeof(te));
				if (oldaction_map_newaction[j] != -1)
					privatenode[i]->actions[oldaction_map_newaction[j]].regret = te[1];
			}
		}
		if (exist) {
			strategy_node** tempprivatenode = new strategy_node * [clusterlen];
			for (int i = 0; i < len; i++) {
				if (oldaction_map_newaction[i] != -1)
					for (int j = 0; j < clusterlen; j++)
						tempprivatenode[j] = privatenode[j]->actions + oldaction_map_newaction[i];
				initialstrategy(tempprivatenode, clusterlen, oldaction_map_newaction[i] != -1);
			}
			delete[] tempprivatenode;
		}
		else {
			for (int i = 0; i < len; i++)
				initialstrategy(NULL, clusterlen, exist);
		}
	}
	else if (len >= 100) {
		if (exist)
			for (int j = 0; j < clusterlen; j++) {
					assert(privatenode[j]->action_len == len);
					assert(privatenode[j]->actions == privatenode[0]->actions);
			}
		if (exist) {
			strategy_node** privatenode2 = new strategy_node * [len];
			for (int j = 0; j < len; j++)
				privatenode2[j] = privatenode[0]->actions + j;
			initialstrategy(privatenode2, len, exist);
			delete[] privatenode2;
		}
		else 
			initialstrategy(NULL, len, exist);
	}
	else throw exception();
}
void initstrategy(strategy_node* root, char filename[]) {//316174
	fin.open(filename, ios::in | ios::binary);
	strategy_node* privatenode[169];
	for (int i = 0; i < 169; i++)
		privatenode[i] = root->actions + i;
	countterminal = 0;
	initialstrategy(privatenode, 169, true);
	cout << "init node:" << countterminal << endl;
	fin.close();
}
void bulid_bluestrategyplay(strategy_nodes* privatenode[], int clusterlen) {
	int len;
	if (fin.eof())
		return;
	fin.read((char*)&len, sizeof(int));
	if (len <= 0) {
		for (int i = 0; i < clusterlen; i++)
			privatenode[i]->action_len = 0;
		countterminal++;
		return;
	}
	else if (len < 100) {
		unsigned char actionstr[12];
		fin.read((char*)actionstr, len);
		for (int i = 0; i < clusterlen; i++) {
			privatenode[i]->init_child(actionstr, len);
			assert(len == privatenode[i]->action_len);
			for (int j = 0; j < len; j++) {
				int te[2];
				fin.read((char*)te, sizeof(te));
				privatenode[i]->actions[j].regret = te[1];
				privatenode[i]->actions[j].averegret[0] = te[1];
			}
			//scale_regret(privatenode[i]->actions, len, 1000000);
		}
		strategy_nodes** tempprivatenode = new strategy_nodes * [clusterlen];
		for (int i = 0; i < len; i++) {
			for (int j = 0; j < clusterlen; j++)
				tempprivatenode[j] = privatenode[j]->actions + i;
			bulid_bluestrategyplay(tempprivatenode, clusterlen);
		}
		delete[] tempprivatenode;
	}
	else if (len >= 100) {
		privatenode[0]->init_chance_node(len);
		for (int j = 1; j < clusterlen; j++) {
			privatenode[j]->action_len = len;
			privatenode[j]->actions = privatenode[0]->actions;
		}
		strategy_nodes** privatenode2 = new strategy_nodes * [len];
		for (int j = 0; j < len; j++)
			privatenode2[j] = privatenode[0]->actions + j;
		bulid_bluestrategyplay(privatenode2, len);
		delete[] privatenode2;
	}
	else throw exception();
}
void loadplay(strategy_nodes* root, char filename[]) {//316174
	fin.open(filename, ios::in | ios::binary);
	root->init_chance_node(169);
	strategy_nodes* privatenode[169];
	for (int i = 0; i < 169; i++)
		privatenode[i] = root->actions + i;
	countterminal = 0;
	bulid_bluestrategyplay(privatenode, 169);
	cout << "countterminal:" << countterminal << endl;
	fin.close();
}
ifstream fin2;
void bulid_bluestrategy2(strategy_nodes* privatenode[], int clusterlen) {
	int len,len2;
	if (fin.eof())
		return;
	fin.read((char*)&len, sizeof(int));
	fin2.read((char*)&len2, sizeof(int));
	assert(len == len2);
	if (len <= 0) {
		for (int i = 0; i < clusterlen; i++)
			privatenode[i]->action_len = 0;
		countterminal++;
		return;
	}
	else if (len < 100) {
		unsigned char actionstr[12], actionstr2[12];
		fin.read((char*)actionstr, len);
		fin2.read((char*)actionstr2, len);
		for (int i = 0; i < len; i++)
			assert(actionstr[i] == actionstr2[i]);
		for (int i = 0; i < clusterlen; i++) {
			privatenode[i]->init_child(actionstr, len);
			for (int j = 0; j < len; j++) {
				int te[2],te2[2];
				fin.read((char*)te, sizeof(te));
				fin2.read((char*)te2, sizeof(te2));
				privatenode[i]->actions[j].regret = privatenode[i]->actions[j].averegret[0] = privatenode[i]->actions[j].averegret[1] = te[1];
				privatenode[i]->actions[j].averegret[2] = te2[1];
			}
		}
		strategy_nodes** tempprivatenode = new strategy_nodes * [clusterlen];
		for (int i = 0; i < len; i++) {
			for (int j = 0; j < clusterlen; j++)
				tempprivatenode[j] = privatenode[j]->actions + i;
			bulid_bluestrategy2(tempprivatenode, clusterlen);
		}
		delete[] tempprivatenode;
	}
	else if (len >= 100) {
		privatenode[0]->init_chance_node(len);
		for (int j = 1; j < clusterlen; j++) {
			privatenode[j]->action_len = len;
			privatenode[j]->actions = privatenode[0]->actions;
		}
		strategy_nodes** privatenode2 = new strategy_nodes * [len];
		for (int j = 0; j < len; j++)
			privatenode2[j] = privatenode[0]->actions + j;
		bulid_bluestrategy2(privatenode2, len);
		delete[] privatenode2;
	}
	else throw exception();
}
void loadplay2(strategy_nodes* root, char filename1[], char filename2[]) {
	fin.open(filename1, ios::in | ios::binary);
	fin2.open(filename2, ios::in | ios::binary);
	root->init_chance_node(169);
	strategy_nodes* privatenode[169];
	for (int i = 0; i < 169; i++)
		privatenode[i] = root->actions + i;
	countterminal = 0;
	bulid_bluestrategy2(privatenode, 169);
	cout << "countterminal:" << countterminal << endl;
	fin2.close();
	fin.close();
}
