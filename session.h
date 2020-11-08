#include <string>
#include <vector>
#include <mutex>

using namespace std;

class Session {
	public:
		Session();
		bool ValidateToken(string token, mutex *lock_);
		string GenerateToken(mutex *lock_);
		static string Random(int len);
		~Session();
	private:
		vector <string> tokens;
};