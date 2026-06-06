const int WMoist = 187;
const int AMoist = 487;
int MLevel;

struct Grow {
  String name;
  int minM;
  int maxM;
};

Grow trees[] = {
  {"Asgave", 20, 40},
  {"Aster", 40, 60},
  {"Bleeding Heart", 41, 60},
  {"Butterfly Weed", 21, 40},
  {"Cactus", 21, 40},
  {"Catmint", 21, 40},
  {"Christmas Fern", 21, 40},
  {"Begonia Coneflower", 21, 40},
  {"Daffodil", 21, 60},
  {"Dalia",  21, 60},
  {"Alpine Current", 21, 40},
  {"Bald Cypress", 21, 80},
  {"Barberry", 21, 40},
  {"Birch", 41, 60},
  {"Crab Apple", 21, 60},
  {"Elderberry", 21, 80},
  {"Elm", 21, 60},
  {"Hydragea", 41, 60},
  {"Potentilla", 10, 40},
  {"Willow", 41, 80},
  {"Apple", 21, 60},
  {"Grapes", 21, 60},
  {"Fig", 21, 60},
  {"Pear", 21, 60},
  {"Peach", 21, 60},
  {"Raspberry", 21, 60},
  {"Strawberry", 21, 60},
  {"Blackberry", 21, 60},
  {"Cranberry", 61, 80}
};

//I got really bored of this project, I could have done a lot more, this was just to practice arrays again

void setup() {
  Serial.begin(9600);
}

void loop() {
  int rawValue = analogRead(A0);
  MLevel = map(rawValue, 187, 487, 100, 0);

  for (Grow i : trees) {
    if (MLevel >= i.minM && MLevel <= i.maxM) {
      Serial.print("This soil can inhabit: ");
      Serial.println(i.name);
    }
  }

  delay(5000);

}