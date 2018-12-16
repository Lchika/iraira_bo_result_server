#ifndef RankingManager_h
#define RankingManager_h

#include <Arduino.h>

typedef struct{
  int time;
  int miss;
  int score;
}rankingData_S;

class RankingManager{
public:
  RankingManager();
  void setData(int time_input, int miss_input);
  void clearData();
  rankingData_S getDataByRank(int rank);
  static const int RANKING_DATA_NUM = 100;  //  保持するデータの数

private:
  static const int MISS_PENALTY = 10;       //  1回のミスで加算されるタイム
  rankingData_S ranking_data[RANKING_DATA_NUM];
  rankingData_S makeRankingData(int time, int miss);
};

#endif
