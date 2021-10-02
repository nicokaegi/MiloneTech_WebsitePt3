git command cheat sheet:

git add "files with changes you want to add"
- let git know what changes you have made
  
git commit -m "some message detailing what changes you made"
- make those changes permenate  
  
git push
- push all your permenate changes to the github
  
git branch 
- show you what branch you are on
  
git branch "name of your branch"
- make a new branch using the name provided
  
git checkout "name of your branch"
- changes what branch you are on (make sure either your changes are commited/saved somewhere, or else will git will stop you from changing branches)
  
git fetch origin "name of a branch"
- get a specific branch from the git hub
  
for the ubuntu VM users:
    
sudo apt install <the name of the package you want>
- type this in the terminal to install pakages like git, and python.

# Introduction
Hello. If you are reading this it's because you will be working on an expansion to the MiloneTech Website.

There will be a learning curve associated with working on this website, as there was for my group when we inherited the project from the previous group. I will do my best to explain how it works, but research will be necessary. There are more than enough resources online to learn what's going on, and trial and error in modifying our code will bolster your understanding immensely. Don't get overwhelmed, continue to study the program, and experiment, and you'll eventually find it to be much easier than it appeared.

This website is built as a REST-ful API. This is perhaps the most important concept to understand: the definition of a REST-ful API is that it's an architectural style that uses GET, PUT, POST, and DELETE requests to communicate and manage data. In our case, this means that the server and the client communicate via these methods. More on how these work below.



# General Structure
There are effectively three main components of the website: the database, the client view, and the server. These effectively form a model, view, controller architecture, where the database forms the structure of the data, the client view is the webpage the user sees, and the server manages the interaction between the two.

The database is an SQL Database
The client view is a set of webpages formatted in HTML and CSS, their functionality programmed in JavaScript
The server is set up on the AWS, and it mostly consists of Python code



# The Database
The database contains all of the information. Quite literally all of it. Account information, sensor readings, account settings, the list goes on. I recommend downloading MySQL Workbench and connecting to it to analyze it's structure. The credentials for connection are in secrets.py. (NOTE only use SELECT commands until you're certain about what you're doing.)

SQL is super easy to learn, and you can pick up 90% of everything you'll use in about an hour. For a free tutorial visit https://www.w3schools.com/sql/

When it comes to querying the database from the server (i.e., in response to GET requests or as part of user data initialization), a Python package called SQLAlchemy is used. This forms a connection to the database and then allows queries to be performed directly from Python. All of the Python files containing database operations are in the dbAPI folder (/Website_Python_Code/flask_website/dbAPI/).



# The Client View
The client view consists of all of the '.html' files. These dictate the layout of the pages. Many of them "extend" other html files using Jinja2, which I will discuss in more detail further down. Essentially this works like extending in any other language. The base page that is extended by others is rendered first, and content from pages that extend that page is filled in in designated places.

In base.html, which most of the pages extend, the CSS style, and multiple scripts, are defined, and as such the rest of the pages that extend base.html also use it's CSS style and can call it's scripts. CSS files match each element in the HTML to entries in them, which contain details about how to display those elements. Whatever specifications are listed are applied to the element on the page.

The client view can most easily communicate with the server by sending GET/POST requests to it. HTML can handle links fairly trivially, but if, for example, you needed to get content without leaving the page, you could associate the clicking of a button (or any other number of actions) with a JavaScript function in which you would send a GET or POST (depending on what you need) request to "usersmilonetech.com/<one of the routes in routes.py>", optionally sending data to the server. The server, in routes.py, would then parse the url, and you could have it parse the data and return it's own data, which you would handle in a function in the JavaScript that receives the response to the request you sent.



# The Server
The server runs on the EC2 virtual machine on AWS. The domain name 'usersmilonetech.com' routes traffic to the IP of the EC2, and then NGINX takes the traffic and redirects it into our Flask application. All of that is more or less automatic, and you're unlikely to need to learn about that. But the next part is important: the Flask application filters the traffic through routes.py, which handles everything appended to the URL, returning the html document that we wish to associate with the url.

For example: a user enters "http://usersmilonetech.com/home" into their address bar. This is a GET request. "usersmilonetech.com" gets rerouted to the IP of the EC2, and the entire url entered gets sent to NGINX. NGINX redirects this to "https://usersmilonetech.com/home" (changes it from http to https), then sends the request to the server that the Flask application is running on (some port on the EC2). routes.py parses the "/login" part of the request - so now the function under @app.route("/home") gets called. (Also note the @login_required, this just requires that the user is logged in to visit this page). This function initializes a bunch of data and then returns a 'render_template' using home.html and the data.

render_templates are an easy way to get custom data into an html document. Normally, when we return an HTML document, it sends a predefined page to the user. The page may need certain data (like the user's name to display in a hello message) from the server. It could get this data by sending a get request in its' JavaScript, but the soonest that could happen is once the page was loaded, and after the time it takes for the script to run. Flask is capable of using render_templates, instead. It uses a 'templating language' called Jinja2 to insert variable data into an HTML page.

So to return to our earlier example, when @app.route("/home") returns render_template('home.html', account_info=current_user.user_data), we can reference account_info in home.html using Jinja2. {{account_info.name}} in the HTML, for instance, would display exactly like current_user.user_data["name"] in the Python code would, as though you had manually written it into the HTML document. Jinja2 can also use if and for statements, giving you immense control over the document you render. You can see this example in effect by examining lines 14-18 of home.html.



# Local Copy Creation
You'll want to create local copies of the website that you can experiment with and completely butcher with no risk of damaging the running website. To do this, perform the following steps:

First Setup:
1. Install Python from the www.python.org
2. Install PyCharm, and create a new project using the "Get from VCS" button. Enter the link to the main page of this repository as a source, and create the project in a folder you'll remember.
3. With the project open in PyCharm, open the terminal window at the bottom of the screen.
4. Type the commands "py -3 -m venv venv", and then "pip install flask" into the terminal
5. Type "venv\Scripts\activate" - there should now be a "(venv)" before the command prompt
6. cd into Website_Python_code
7. Type "run.py"
8. It will eventually throw an error saying you're missing a package
9. Type "pip install <the package it mentioned>"
10. Repeat steps 7-9 (even for email_validator), until it says: "running on http://127.0.0.1:5000"
11. Follow the link it provides to verify that a version of the website is running on your local machine
12. Press ctrl+C in the terminal when you wish to shut it down

To Launch the Website After:
1. Open PyCharm
2. If the (venv) is no longer present, type "venv\Scripts\activate", but PyCharm tends to keep it activated
3. Type "set FLASK_DEBUG=true" to allow changes you make to take effect without restarting
4. Enter Website_Python_Code and type "run.py"

The local copies pull from and modify the same database, so it's like having a complete copy of the website.



# TO-DO
These are things we did not get working properly:
1. Have PayPal transactions update the database's accountStatus field. The PayPal interface is managed in settings.html. From there, you will need to send a POST request to the server upon transaction completion, and catch it somewhere in routes.py. Note that this request will need some kind of verification code from PayPal included in it, so that users can't engineer their own POST requests and upgrade their payment status. The Python code will need to somehow verify based on that code that the payment is legitimate. If it is, then you can perform a database operation to update the account status accordingly.
2. Hook up PayPal to Chris' business account. Right now it uses sandbox, a testing API. Unfortunately I can't offer much advice on this one. I would recommend assigning at least one person to specialize in working with PayPal's API, who should ensure that it's all working securely and properly.
3. Implement live updates to the charts. I had previously done this with SocketIO but that's like using a blowtorch to light a candle, and I recommend removing SocketIO entirely. My suggestion is implementing a client-side JavaScript script that periodically performs a GET on the server that checks for new data. Some of the functionality for adding datapoints to the graph is already there. If you use those functions with properly formatted data from the GET request response, it's already set up to update the graphs. Feel free to modify the add_datapoint() function in the sensor html pages as necessary. You may also want to move it to something that all of the sensor pages extend, because right now there are a few copies of the add_datapoint function, some of which are slightly different, in different html files.

Best of luck.
If you need advice I will give what I can.

-Nicholas Anderson

Phone: 609 923 4024

Email: nicholasanderson940@yahoo.com
