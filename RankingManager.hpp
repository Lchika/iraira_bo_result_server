#ifndef RankingManager_h
#define RankingManager_h

#include <map>
#include <functional>
#include <Arduino.h>

typedef struct{
  int time;
  int miss;
  int score;
}rankingData_S;

typedef struct{
  std::map<String, std::function<void(rankingData_S rankData[], int length)>> call_backs;
}initParam;

class RankingManager{
public:
  RankingManager();
  RankingManager(initParam init_param);
  void setData(int time_input, int miss_input);
  void clearData();
  rankingData_S getDataByRank(int rank);
  static const int RANKING_DATA_NUM = 100;  //  保持するデータの数

private:
  static const int MISS_PENALTY = 10;       //  1回のミスで加算されるタイム
  rankingData_S ranking_data[RANKING_DATA_NUM];
  rankingData_S makeRankingData(int time, int miss);
  std::map<String, std::function<void(rankingData_S rankData[], int length)>> _call_backs;
};

#endif
