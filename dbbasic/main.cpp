#include <assert.h>
#include <iostream>
#include <sstream>

#include "SFML/Graphics.hpp"
#include "Utils.h"
#include "..\..\sqlite\sqlite3.h"

using namespace sf;
using namespace std;

static int PrintToConsole(void* NotUsed, int argc, char** argv, char** azColName) {
	stringstream ss;
	int i;
	for (i = 0; i < argc; i++) {
		ss << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
	}
	DebugPrint(ss.str().c_str(), "");
	return 0;
}

void RunQuery(const string title, const string& sql, sqlite3& db)
{
	DebugPrint(title);
	DebugPrint("\nRun SQL=> ", sql);
	char* zErrMsg = 0;
	int rc = sqlite3_exec(&db, sql.c_str(), PrintToConsole, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		DebugPrint("SQL error: ", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		DebugPrint("Finished OK");
	}
}


int main()
{
	// Create the main window
	RenderWindow window(VideoMode(1200, 800), "database");
	sf::Font font;
	if (!font.loadFromFile("data/fonts/comic.ttf"))
		assert(false);

	DebugPrint("Database starter app");

	//create or open the database
	sqlite3* db;
	int rc = sqlite3_open("data/chinook.db", &db);
	if (rc) {
		DebugPrint("Can't open database: ", sqlite3_errmsg(db));
		assert(false);
	}

	char* zErrMsg = 0;
	rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, 0, &zErrMsg);
	if (rc != SQLITE_OK) {
		DebugPrint("SQL error: ", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	//Display the names of all tables, title "Tables in Chinook"

	RunQuery("Tables in Chinook", "SELECT name FROM sqlite_master WHERE type = 'table'", *db);

	//Display all the field names in the invoices table, title “Fields in Customers” 

	RunQuery("Fields in Customers", "SELECT name FROM pragma_table_info('invoices')", *db);

	//Display all the customer information on file for someone with name “Julia Barnett”,
	//title “Customer info for Julia Barnett”, format “FIELD_NAME – DATA”

	RunQuery("Customer info for Julia Barnett", "SELECT * FROM CUSTOMERS WHERE FirstName = \"Julia\" AND LastName = \"Barnett\"", *db);

/*
* Display everything that a customer called Julia Barnett bought, title “Money spent by Julia”
* formatted as “£9.99 – DATE”. This requires a join.
* At the end display the total Julia has spent, format “Final total=£9.99”. [£9.99 is just an example, it should add up to roughly £43.
* So you aren’t listing individual music titles, just invoice totals with dates.]
*/
	RunQuery("Money spent by Julia", "SELECT Total, InvoiceDate FROM customers INNER JOIN Invoices ON Customers.CustomerID = Invoices.CustomerID WHERE FirstName = \"Julia\" AND LastName = \"Barnett\" UNION SELECT SUM(Total) as `Final Total`, NULL FROM customers INNER JOIN invoices ON Customers.CustomerID = Invoices.CustomerID WHERE FirstName = \"Julia\" AND LastName = \"Barnett\"", *db);

/*
Display how much every customer has spent, title “How much each customer has been charged”, format “£9.99 – FIRST_NAME LAST_NAME”. 
Order this list from biggest spender to smallest spender. This requires a sum, a join, group_by and order_by. 
At the end display the total income for the company, format “Final total=£9.99”.
*/

	RunQuery("How much each customer has been charged", "SELECT SUM(Total) as SumTotal, FirstName, LastName FROM Invoices INNER JOIN Customers ON Invoices.CustomerID = Customers.CustomerID GROUP BY Customers.CustomerID ORDER BY SumTotal DESC", *db);

	Clock clock;
	// Start the game loop 
	while (window.isOpen())
	{
		bool fire = false;
		// Process events
		Event event;
		while (window.pollEvent(event))
		{
			// Close window: exit
			if (event.type == Event::Closed) 
				window.close();
		} 

		// Clear screen
		window.clear();

		float elapsed = clock.getElapsedTime().asSeconds();
		clock.restart();

		sf::Text txt("Database starter app", font, 30);
		window.draw(txt);


		// Update the window
		window.display();
	}

	return EXIT_SUCCESS;
}
