/*
 * FannieMae.cpp
 *      Author: Emily Singleton and Elena Oh
 *
 */


#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>

using namespace pqxx;
using namespace std;


const string originalInt = "original_interest_rate", unpaidBal = "upb", loanToVal = "ltv", combLoanToVal = "cltv", debtToInc = "dti", credScore = "credit_score";

/*
 * Returns a vector of unique states in alphabetical order from the given year and quarter
 */
vector<string> findStates(int year, int quarter){
	string sql;
	string v1;
	vector<string> states;

	try{
		connection C("dbname=fanniemae user=fanniemae_user password=hfsl_fannimae hostaddr=155.246.103.74 port=5432");
		if (C.is_open())
		{
			cout << "Connected to database: " << C.dbname() << endl;

		}
		else
		{
			cout << "Cannot Open Database " << endl;
			return states;
		}

		//Turns int parameters into string for sql queries
		string yearString = to_string(year);
		string quarterString = "Q" + to_string(quarter);

		sql = "SELECT DISTINCT property_state FROM acquisitions.\"acquisition" + yearString + quarterString + "\" ORDER BY property_state ASC;";
		nontransaction N(C);

		result R( N.exec( sql ));
		for(result::const_iterator c = R.begin(); c != R.end(); c++){
			c[0].to(v1);
			states.push_back(v1);
		}


		C.disconnect();
	}
	catch(const exception &e)
	{
		cerr << e.what() << endl;
	}
	return states;
}


/*
 * Returns a vector of different statistics based on a given year, quarter and column
 * First two are each of the means, third and forth are each of the standard deviations
 * Lastly is the correlation between the two variables
 */
vector<double> sql(int year, int quarter, string state, string column1, string column2){
	//Variables will hold the SQL query if the database successfully connects
	string count;
	string mean1;
	string mean2;
	string stdev1;
	string stdev2;
	string corr;

	vector<double> dataPoints;

	try{
		connection C("dbname=fanniemae user=fanniemae_user password=hfsl_fannimae hostaddr=155.246.103.74 port=5432");
		if (C.is_open())
		{
			cout << "Connected to database: " << C.dbname() << endl;

		}
		else
		{
			cout << "Cannot Open Database " << endl;
			return dataPoints;
		}

		//Turns int parameters into string for sql queries
		string yearString = to_string(year);
		string quarterString = "Q" + to_string(quarter);

		if(state == "none"){
			count = "SELECT Count(DISTINCT loan_id) FROM acquisitions.\"acquisition" + yearString + quarterString + "\";";
			mean1 = "SELECT avg(" + column1 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\";";
			mean2 = "SELECT avg(" + column2 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\";";
			stdev1 = "SELECT stddev_pop(" + column1 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\";";
			stdev2 = "SELECT stddev_pop(" + column2 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\";";
			corr = "SELECT corr(" + column1 + ", " + column2 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\";";
		}
		else{
			count = count = "SELECT Count(DISTINCT loan_id) FROM acquisitions.\"acquisition" + yearString + quarterString + "\" WHERE property_state = '" + state + "';";
			mean1 = "SELECT avg(" + column1 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\" WHERE property_state = '" + state + "';";
			mean2 = "SELECT avg(" + column2 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\" WHERE property_state = '" + state + "';";
			stdev1 = "SELECT stddev_pop(" + column1 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\" WHERE property_state = '" + state + "';";
			stdev2 = "SELECT stddev_pop(" + column2 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\" WHERE property_state = '" + state + "';";
			corr = "SELECT corr(" + column1 + ", " + column2 + ") FROM acquisitions.\"acquisition" + yearString + quarterString + "\" WHERE property_state = '" + state + "';";
		}

		nontransaction N(C);

		//Executing the SQL queries
		result countResult( N.exec( count ));
		result mean1Result( N.exec( mean1 ));
		result mean2Result( N.exec( mean2 ));
		result stdev1Result( N.exec( stdev1 ));
		result stdev2Result( N.exec( stdev2 ));
		result corrResult( N.exec( corr ));

		double holder;
		result::const_iterator iterator = countResult.begin();
		iterator[0].to(holder);
		dataPoints.push_back(holder);

		//Inserting mean1 into the vector that is returned
		iterator = mean1Result.begin();
		iterator[0].to(holder);
		dataPoints.push_back(holder);

		//Inserting mean2 into the vector that is returned
		iterator = mean2Result.begin();
		iterator[0].to(holder);
		dataPoints.push_back(holder);

		//Inserting stdev1 into the vector that is returned
		iterator = stdev1Result.begin();
		iterator[0].to(holder);
		dataPoints.push_back(holder);

		//Inserting stdev2 into the vector that is returned
		iterator = stdev2Result.begin();
		iterator[0].to(holder);
		dataPoints.push_back(holder);

		//Inserting corr in the vector that is returned
		iterator = corrResult.begin();
		iterator[0].to(holder);
		dataPoints.push_back(holder);

		C.disconnect();
	}
	catch(const exception &e)
	{
		cerr << e.what() << endl;
	}
	return dataPoints;
}

struct stats{
	//Makes and SQL call to get vector of the column
	int year;
	int quarter;
	string state;
	string column1;
	string column2;
	stats(int y, int q, string s, string c1, string c2):
		year(y),
		quarter(q),
		state(s),
		column1(c1),
		column2(c2)
	{}

	vector<double> dataPoints = sql(year, quarter, state, column1, column2);

	//Statistics
	const double count = dataPoints[0];
	const double meanVal1 = dataPoints[1];
	const double meanVal2 = dataPoints[2];
	const double stdevVal1 = dataPoints[3];
	const double stdevVal2 = dataPoints[4];
	const double corr = dataPoints[5];

};



int main()
{
	//Asks the user for a year
	int year;
	while (((cout << "Please select a year between 2000 and 2015: ") && !(cin >> year))  || (year < 2000) || (year > 2015) || cin.peek() != '\n')
	{
		cout << "Value must be an integer between 2000 and 2015" << std::endl;
		cin.clear();
		cin.ignore();
		cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	cout << "*******************************************" << endl;

	//Asks the user for a quarter
	//The year 2015 only has data for quarter one and quarter two so checks for that case
	//All the other years have four quarters of data
	int quarter;
	if(year < 2015){
		while (((cout << "Please select a quarter (1, 2, 3 or 4): ") && !(cin >> quarter))  || (quarter < 1) || (quarter > 4) || cin.peek() != '\n')
		{
			cout << "Value must be an integer between 1 and 4" << std::endl;
			cin.clear();
			cin.ignore();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
	else{
		while (((cout << "Please select a quarter (1 or 2): ") && !(cin >> quarter))  || (quarter < 1) || (quarter > 2) || cin.peek() != '\n')
		{
			cout << "Value must be an integer between 1 and 2" << std::endl;
			cin.clear();
			cin.ignore();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
	cout << "*******************************************" << endl;

	//Ask the user if they would like to restrict the data by a certain state
	int stateYN;
	while (((cout << "Would you like to restrict the data by state? (1 for yes & 2 for no): ") && !(cin >> stateYN))  || (stateYN < 1) || (stateYN > 2) || cin.peek() != '\n')
	{
		cout << "Value must be an integer 1 or 2" << std::endl;
		cin.clear();
		cin.ignore();
		cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	//If they choose to restrict the data by a state then they enter this if statement
	//They are then presented with a list of available states from which they must select one
	vector<string> availStates;
	string chosenState;
	if(stateYN == 1){
		//Pull all state names & ask for state
		availStates = findStates(year, quarter);
		//Prints out states
		for(int i = 0; i < availStates.size(); i++){
			cout << i+1 << ": " << availStates[i] << endl;
		}
		//Asks the user to choose a state
		int stateChoice;
		cout << "Select one of the following states by its corresponding number: ";
		while ((!(cin >> stateChoice))  || (stateChoice < 1) || (stateChoice > availStates.size()) || cin.peek() != '\n')
		{
			cout << "Value must be an integer between 1 and " << availStates.size() << " \nTry again: ";
			cin.clear();
			cin.ignore();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
		chosenState = availStates[stateChoice - 1];
		cout << "You chose state: " << chosenState << endl;
	}
	else
		chosenState = "none";
	cout << "*******************************************" << endl;


	//Asks the user to choose which variable they would like to run statistics on
	cout << "Please choose a variable that you would like to run statistics on \n" <<
			"1 - Original Interest Rate\n" << "2 - Unpaid Balance\n" << "3 - Loan to Value\n" <<
			"4 - Combined Loan to Value\n" << "5 - Debt to Income\n" << "6 - Credit Score\n" <<
			"Type in the number that corresponds with the column to select it: ";
	int choiceNum1;
	string column1;
	while ((!(cin >> choiceNum1))  || (choiceNum1 < 1) || (choiceNum1 > 6) || cin.peek() != '\n')
		{
			cout << "Value must be an integer between 1 and 6\nTry again: ";
			cin.clear();
			cin.ignore();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	cout << "*******************************************" << endl;
	//Assigns the column given their numerical choice
	if(choiceNum1 == 1){
		column1 = originalInt;
	}
	else if (choiceNum1 == 2){
		column1 = unpaidBal;
	}
	else if (choiceNum1 == 3){
		column1 = loanToVal;
	}
	else if (choiceNum1 == 4){
		column1 = combLoanToVal;
	}
	else if (choiceNum1 == 5){
		column1 = debtToInc;
	}
	else{
		column1 = credScore;
	}

	//Asks the user for another variable that they would like to run statistcs on
	cout << "Choose another column to find correlation with: ";
	int choice2;
	string column2;
	while ((!(cin >> choice2))  || (choice2 < 1) || (choice2 > 6) || cin.peek() != '\n')
	{
		cout << "Value must be an integer between 1 and 6\nTry again: ";
		cin.clear();
		cin.ignore();
		cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	cout << "*******************************************" << endl;
	//Assigns the second column based off of user's choice
	if(choice2 == 1){
		column2 = originalInt;
	}
	else if (choice2 == 2){
		column2 = unpaidBal;
	}
	else if (choice2 == 3){
		column2 = loanToVal;
	}
	else if (choice2 == 4){
		column2 = combLoanToVal;
	}
	else if (choice2 == 5){
		column2 = debtToInc;
	}
	else{
		column2 = credScore;
	}

	//Creates the struct from which the statistics are returned and printed out
	stats usersStats(year, quarter, chosenState, column1, column2);
	cout << "Statistics for " << column1 << " & " << column2;
	if(chosenState != "none"){
		cout << " from " << chosenState << endl;
	}
	else
		cout << endl;
	cout << "Number of loans: " << usersStats.count << endl;
	cout << "Mean for " << column1 << ": "<< usersStats.meanVal1 << endl;
	cout << "Mean for " << column2 << ": "<< usersStats.meanVal2 << endl;
	cout << "Standard Deviation for " << column1 << ": " << usersStats.stdevVal1 << endl;
	cout << "Standard Deviation for " << column2 << ": " << usersStats.stdevVal2 << endl;
	cout << "Correlation between " << column1 << " & " << column2 << ": "<< usersStats.corr << endl;

	return 0;
}

