#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>

using namespace std;

class Exception {
protected:
	string text;
	int line;
	string source;
	string time;

public:
	Exception(const string& text, int line, const string& source, const string& time)
		: text(text), line(line), source(source), time(time) {}

	virtual ~Exception() {}

	string getText() const { return text; }
	int getLine() const { return line; }
	string getSource() const { return source; }
	string getTime() const { return time; }
};

class QuizException : public Exception {
public:
	QuizException(const string& text, int line, const string& source, const string& time)
		: Exception(text, line, source, time) {}
};

class InvalidArgumentException : public Exception {
public:
	InvalidArgumentException(const string& text, int line, const string& source, const string& time)
		: Exception(text, line, source, time) {}
};

void shuffleIndexes(vector<int>& indexes) {
	for (int i = (int)indexes.size() - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		int temp = indexes[i];
		indexes[i] = indexes[j];
		indexes[j] = temp;
	}
}

string readLine(const string& prompt) {
	cout << prompt;
	string line;
	getline(cin, line);
	return line;
}

int readInt(const string& prompt, int min, int max) {
	while (true) {
		cout << prompt;
		int value;
		cin >> value;
		if (cin.fail() || value < min || value > max) {
			cin.clear();
			cin.ignore(1000, '\n');
			cout << "Enter a number from " << min << " to " << max << endl;
		}
		else {
			cin.ignore(1000, '\n');
			return value;
		}
	}
}

class Question {
	string text;
	vector<string> answers;
	int correct;

public:
	Question(const string& text) : text(text), correct(-1) {}

	const string& getText() const { return text; }
	const string& getAnswer(int index) const { return answers[index]; }
	int getAnswerCount() const { return (int)answers.size(); }

	void addAnswer(const string& answer) { answers.push_back(answer); }
	void setCorrect(int index) { correct = index; }

	bool isCorrect(int chosen) const { return chosen == correct; }

	void validate() const {
		if (text.empty()) {
			throw InvalidArgumentException("Question text cannot be empty", __LINE__, __FILE__, __TIME__);
		}
		if (answers.size() < 2 || answers.size() > 4) {
			throw InvalidArgumentException("Question needs 2 to 4 answers", __LINE__, __FILE__, __TIME__);
		}
		if (correct < 0 || correct >= (int)answers.size()) {
			throw InvalidArgumentException("Correct answer is not valid", __LINE__, __FILE__, __TIME__);
		}
	}
};

class Quiz {
	string name;
	vector<Question> questions;

public:
	Quiz() : name("") {}
	Quiz(const string& name) : name(name) {}

	const string& getName() const { return name; }
	int getQuestionCount() const { return (int)questions.size(); }
	const Question& getQuestion(int index) const { return questions[index]; }

	void addQuestion(const Question& q) { questions.push_back(q); }
};

class QuizDatabase {
	map<string, Quiz> quizzes;

public:
	void addQuiz(const Quiz& quiz) {
		quizzes[quiz.getName()] = quiz;
	}

	bool hasQuiz(const string& name) const {
		return quizzes.count(name) > 0;
	}

	bool isEmpty() const { return quizzes.empty(); }

	vector<string> getQuizNames() const {
		vector<string> names;
		for (map<string, Quiz>::const_iterator it = quizzes.begin(); it != quizzes.end(); ++it) {
			names.push_back(it->first);
		}
		return names;
	}

	Quiz& getQuiz(const string& name) {
		map<string, Quiz>::iterator it = quizzes.find(name);
		if (it == quizzes.end()) {
			throw QuizException("Quiz not found", __LINE__, __FILE__, __TIME__);
		}
		return it->second;
	}
};

class Result {
	string playerName;
	string quizName;
	int correct;
	int incorrect;
	int pass;

public:
	Result(const string& playerName, const string& quizName, int correct, int incorrect, int pass)
		: playerName(playerName), quizName(quizName), correct(correct), incorrect(incorrect), pass(pass) {}

	const string& getPlayerName() const { return playerName; }
	const string& getQuizName() const { return quizName; }
	int getCorrect() const { return correct; }
	int getIncorrect() const { return incorrect; }
	int getPass() const { return pass; }

	void show() const {
		cout << endl << "===== Result =====" << endl;
		cout << "Player:    " << playerName << endl;
		cout << "Quiz:      " << quizName << endl;
		cout << "Correct:   " << correct << endl;
		cout << "Incorrect: " << incorrect << endl;
		cout << "Pass:      " << pass << " (skipped)" << endl;
	}
};

class ScoreBoard {
	vector<Result> results;

public:
	void addResult(const Result& result) {
		results.push_back(result);
	}

	void show() const {
		cout << endl << "===== Leader Board (Top 10) =====" << endl;
		if (results.empty()) {
			cout << "No results yet." << endl;
			return;
		}

		vector<Result> sorted = results;
		int shown = 0;
		for (int i = 0; i < (int)sorted.size() && shown < 10; i++) {
			int best = i;
			for (int j = i + 1; j < (int)sorted.size(); j++) {
				if (sorted[j].getCorrect() > sorted[best].getCorrect()) {
					best = j;
				}
			}
			Result temp = sorted[i];
			sorted[i] = sorted[best];
			sorted[best] = temp;

			shown++;
			cout << shown << ") " << sorted[i].getPlayerName()
				<< " - " << sorted[i].getQuizName()
				<< " - Correct: " << sorted[i].getCorrect() << endl;
		}
	}
};

class QuizSession {
	string playerName;
	Quiz quiz;
	vector<int> questionOrder;
	vector<vector<int>> answerOrder;
	vector<int> selected;
	int current;

public:
	QuizSession(const string& playerName, const Quiz& quiz, bool randomQuestions, bool randomAnswers)
		: playerName(playerName), quiz(quiz), current(0) {
		int n = quiz.getQuestionCount();

		for (int i = 0; i < n; i++) {
			questionOrder.push_back(i);
		}
		if (randomQuestions) {
			shuffleIndexes(questionOrder);
		}

		answerOrder.resize(n);
		selected.assign(n, -1);
		for (int i = 0; i < n; i++) {
			int count = quiz.getQuestion(i).getAnswerCount();
			for (int j = 0; j < count; j++) {
				answerOrder[i].push_back(j);
			}
			if (randomAnswers) {
				shuffleIndexes(answerOrder[i]);
			}
		}
	}

	int getCurrentAnswerCount() const {
		int qi = questionOrder[current];
		return (int)answerOrder[qi].size();
	}

	void showCurrent() const {
		int qi = questionOrder[current];
		const Question& q = quiz.getQuestion(qi);

		cout << endl << "--- Question " << current + 1 << " of " << questionOrder.size() << " ---" << endl;
		cout << q.getText() << endl;

		for (int d = 0; d < (int)answerOrder[qi].size(); d++) {
			int original = answerOrder[qi][d];
			cout << "  " << d + 1 << ") " << q.getAnswer(original);
			if (selected[qi] == original) {
				cout << "   [selected]";
			}
			cout << endl;
		}
	}

	void select(int displayIndex) {
		int qi = questionOrder[current];
		selected[qi] = answerOrder[qi][displayIndex];
	}

	bool next() {
		if (current + 1 < (int)questionOrder.size()) {
			current++;
			return true;
		}
		return false;
	}

	bool previous() {
		if (current > 0) {
			current--;
			return true;
		}
		return false;
	}

	Result submit() const {
		int correct = 0;
		int incorrect = 0;
		int pass = 0;

		for (int i = 0; i < (int)questionOrder.size(); i++) {
			int qi = questionOrder[i];
			if (selected[qi] == -1) {
				pass++;
			}
			else if (quiz.getQuestion(qi).isCorrect(selected[qi])) {
				correct++;
			}
			else {
				incorrect++;
			}
		}
		return Result(playerName, quiz.getName(), correct, incorrect, pass);
	}
};

class Menu {
	QuizDatabase database;
	ScoreBoard board;
	bool isAdmin;

public:
	Menu() : isAdmin(false) {}

	void run() {
		if (!login()) {
			return;
		}

		bool running = true;
		while (running) {
			cout << endl << "===== Main Menu " << (isAdmin ? "(Admin)" : "(Guest)") << " =====" << endl;
			cout << "1) Create quiz" << endl;
			cout << "2) Start quiz" << endl;
			cout << "3) Leader board" << endl;
			cout << "0) Exit" << endl;
			int choice = readInt("Choose: ", 0, 3);

			try {
				if (choice == 1) {
					createQuiz();
				}
				else if (choice == 2) {
					startQuiz();
				}
				else if (choice == 3) {
					board.show();
				}
				else {
					running = false;
				}
			}
			catch (const Exception& ex) {
				printException(ex);
			}

			if (running) {
				readLine("Press Enter to continue...");
			}
		}
		cout << "Goodbye!" << endl;
	}

private:
	void printException(const Exception& ex) const {
		cout << "Error: " << ex.getText() << endl;
		cout << "Line: " << ex.getLine() << endl;
		cout << "Source: " << ex.getSource() << endl;
		cout << "Time: " << ex.getTime() << endl;
	}

	bool login() {
		string adminPassword = "admin123";

		while (true) {
			cout << endl << "===== Login =====" << endl;
			cout << "1) Admin login" << endl;
			cout << "2) Guest login" << endl;
			cout << "0) Exit" << endl;
			int choice = readInt("Choose: ", 0, 2);

			if (choice == 0) {
				return false;
			}
			if (choice == 2) {
				isAdmin = false;
				return true;
			}

			string password = readLine("Admin password: ");
			if (password == adminPassword) {
				isAdmin = true;
				return true;
			}
			cout << "Wrong password." << endl;
		}
	}

	void createQuiz() {
		if (!isAdmin) {
			throw QuizException("Only Admin can create quizzes", __LINE__, __FILE__, __TIME__);
		}

		string name = readLine("Enter quiz name: ");
		if (name == "") {
			throw InvalidArgumentException("Quiz name cannot be empty", __LINE__, __FILE__, __TIME__);
		}
		if (database.hasQuiz(name)) {
			throw QuizException("A quiz with this name already exists", __LINE__, __FILE__, __TIME__);
		}

		Quiz quiz(name);
		while (true) {
			cout << endl << "Quiz " << name << " has " << quiz.getQuestionCount() << " question(s)" << endl;
			cout << "1) New question" << endl;
			cout << "2) Back (discard)" << endl;
			cout << "3) Save quiz" << endl;
			int choice = readInt("Choose: ", 1, 3);

			if (choice == 1) {
				Question q = buildQuestion();
				try {
					q.validate();
					quiz.addQuestion(q);
					cout << "Question added." << endl;
				}
				catch (const Exception& ex) {
					cout << "Question not added: " << ex.getText() << endl;
				}
			}
			else if (choice == 2) {
				return;
			}
			else {
				if (quiz.getQuestionCount() == 0) {
					cout << "Add at least one question before saving." << endl;
				}
				else {
					database.addQuiz(quiz);
					cout << "Quiz saved." << endl;
					return;
				}
			}
		}
	}

	Question buildQuestion() {
		Question q(readLine("Question text: "));

		while (q.getAnswerCount() < 4) {
			string answer = readLine("Answer " + to_string(q.getAnswerCount() + 1) + " (blank to finish): ");
			if (answer == "") {
				break;
			}
			q.addAnswer(answer);
		}

		if (q.getAnswerCount() >= 2) {
			int correct = readInt("Which answer is correct (1-" + to_string(q.getAnswerCount()) + "): ",
				1, q.getAnswerCount());
			q.setCorrect(correct - 1);
		}
		return q;
	}

	void startQuiz() {
		if (database.isEmpty()) {
			throw QuizException("No quizzes yet. Admin must create one first", __LINE__, __FILE__, __TIME__);
		}

		string player = readLine("Enter your name: ");
		if (player == "") {
			throw InvalidArgumentException("Name cannot be empty", __LINE__, __FILE__, __TIME__);
		}

		vector<string> names = database.getQuizNames();
		cout << endl << "Available quizzes:" << endl;
		for (int i = 0; i < (int)names.size(); i++) {
			cout << "  " << i + 1 << ") " << names[i] << endl;
		}
		int pick = readInt("Select quiz: ", 1, (int)names.size()) - 1;

		bool randomQuestions = readInt("Random question order? 1) Yes 2) No: ", 1, 2) == 1;
		bool randomAnswers = readInt("Random answer order? 1) Yes 2) No: ", 1, 2) == 1;

		QuizSession session(player, database.getQuiz(names[pick]), randomQuestions, randomAnswers);
		Result result = runSession(session);

		board.addResult(result);
		result.show();
	}

	Result runSession(QuizSession& session) {
		while (true) {
			session.showCurrent();
			string command = readLine("Enter 1-" + to_string(session.getCurrentAnswerCount())
				+ " to answer, P previous, N next, S submit: ");

			if (command == "p" || command == "P") {
				if (!session.previous()) {
					cout << "This is the first question." << endl;
				}
			}
			else if (command == "n" || command == "N") {
				if (!session.next()) {
					cout << "This is the last question." << endl;
				}
			}
			else if (command == "s" || command == "S") {
				return session.submit();
			}
			else if (command.size() == 1 && command[0] >= '1'
				&& command[0] - '0' <= session.getCurrentAnswerCount()) {
				session.select(command[0] - '1');
				cout << "Answer saved." << endl;
			}
			else {
				cout << "Invalid choice." << endl;
			}
		}
	}
};

int main() {
	srand((unsigned int)time(0));

	Menu menu;
	menu.run();

	system("pause");
	return 0;
}