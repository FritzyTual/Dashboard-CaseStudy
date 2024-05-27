If you need to configure settings such as the database, web title, or members, go to "server_config.json."

MongoDB Installation Guide:
---------------------------
* 1. Obtain the MongoDB URI from https://cloud.mongodb.com/v2/66461b3140129f1306fc0952#/clusters
* 2. Connect
* 3. Drivers
* 4. Resources ---> Access your Database Users
* 5. Edit your password to your choice

* 6. Go to Data API -> enable it (it will be used as an API Endpoint to make the Microcontroller connect and insert data into the database).
* 7. Go to Test Your API
* 8. Generate API Key (it will be used for the connectivity of the microcontroller to MongoDB).

Software Side Installation Guide:
---------------------------------
* 1. Install Node.js at https://nodejs.org/en/download/package-manager
* 2. Create a folder at any location of your choice
* 3. Open CMD and type -> cd (space)
* 4. Drag the folder you created into CMD, CMD will look something like this: cd C:\ems\YourFolderName
* 5. Press Enter, then type -> npm init -y
* 6. After that, type -> npm install express mongodb ejs
* 7. Go to server_config.js [configure your data according to your MongoDB account]
* 8. After configuration, you may now type: npm start
* 9. Access your dashboard at http://127.0.0.1:3000/

© Bernadette, Fritzy Mae
