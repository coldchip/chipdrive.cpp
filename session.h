#include <string>
#include <vector>
#include <mutex>

using namespace std;

typedef struct _Token {
	string id;
	int expiry;
} Token;

class Session {
	public:
		Session();
		bool ValidateToken(string token, mutex *lock_);
		string GenerateToken(mutex *lock_);
		static string Random(int len);
		~Session();
	private:
		vector<Token> tokens;
};