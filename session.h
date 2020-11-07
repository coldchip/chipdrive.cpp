#include <string>
#include <vector>

using namespace std;

class Session {
	public:
		Session();
		bool ValidateToken(string token);
		string GenerateToken();
		static string Random(int len);
		~Session();
	private:
		vector <string> tokens;
};