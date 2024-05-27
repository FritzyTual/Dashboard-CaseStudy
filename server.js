const config = require('./server_config') // configuration aha ga save ang mga data needed for visual ease
const express = require('express') // express js
const { MongoClient, ServerApiVersion, ObjectId } = require('mongodb') // initialize ang mongodb server

let uri = config.database.db_uri // ang URI sa mongodb cluster // cluster = a group of similar things or people positioned or occurring closely together.

const app = express() // initialize express server

app.set('view engine', 'ejs') // initialize ang client-side which is murag html
app.use(express.static('public'))

/*
 * @Description: Asynchronous connection to mongodb to provide data collections collected from the hardware side
 */
async function retrieveDataCollections()
{
	const client = new MongoClient(uri) // initialize mongodb connection
    try
	{
        await client.connect() // connect to mongodb
        console.log("Connected to MongoDB Atlas")

        const database = client.db(config.database.db_mongodb_database) // initialize database, inside client.db(the configuration initialized)
        const collection = database.collection(config.database.db_mongodb_collection) // initialize data collections, same goes database.collection in specified set of data na

		// start and end of today
        const startOfToday = new Date();
        startOfToday.setHours(0, 0, 0, 0); // set time to the start of the day
        const endOfToday = new Date();
        endOfToday.setHours(23, 59, 59, 999); // set time to the end of the day

		// filter data only today
        const cursor = collection.find(
		{
            _id:
			{
                $gte: ObjectId.createFromTime(startOfToday.getTime() / 1000), // greater than or equal, to filter documents by their _id timestamp, ensuring only documents created today are returned
                $lt: ObjectId.createFromTime(endOfToday.getTime() / 1000) // less than
            }
        })
		// const cursor = collection.find({}) // retrieves all data collections

        const documents = await cursor.toArray() // convert cursor to array of documents

        return documents // return documents to use for the client-side retrieval.
    } finally {
        await client.close() // close the client connection to prevent resource leaks
        console.log("Connection to MongoDB Atlas closed")
    }
}

/*
 * @Description: initialize homepage and render the documents from retrieveDataCollections(), as well as the config.
 */
app.get('/', async (req, res) =>
{
    const documents = await retrieveDataCollections()
    res.render('index', { documents, config })
})	

/*
 * @Description: listens to port number which will the server can be opened through browser
 */
app.listen(config.server_port, () => {
    console.log(`Server is running on port ${config.server_port}`)
})

/*
 * @Description: used for new data retrieval that gets inserted to the mongodb database, and then retrieve as a set of new documents.
 */
app.get('/latestData', async (req, res) =>
{
    const documents = await retrieveDataCollections()
    res.json(documents)
})

/*
 * @Description: deletes a specific data throughout the collections.
 */
app.delete('/delete/:id', async (req, res) =>
{
    const id = req.params.id // intialize the specific data that is to be deleted by the user
    const client = new MongoClient(uri) // connect to mongodb again because it is set to be closed after usage to prevent resource leaks.
    try
	{
        await client.connect()
        const database = client.db(config.database.db_mongodb_database)
        const collection = database.collection(config.database.db_mongodb_collection)

        const result = await collection.deleteOne({ _id: new ObjectId(id) }) // the specified id to delete then
        if (result.deletedCount === 1)
            res.status(200).send({ message: 'Data successfully deleted' })
        else
            res.status(404).send({ message: 'Data not found' })
    } catch (error) {
        res.status(500).send({ message: 'An error occurred when deleting data', error: error.message })
    } finally {
        await client.close()
    }
})

/*
 * @Description: deletes all data throughout the collections.
 */
app.delete('/deleteAll', async (req, res) =>
{
    const client = new MongoClient(uri)
    try
	{
        await client.connect()
        const database = client.db(config.database.db_mongodb_database)
        const collection = database.collection(config.database.db_mongodb_collection)

        const result = await collection.deleteMany({})
        if (result.deletedCount > 0)
            res.status(200).send({ message: 'All data successfully deleted' })
        else
            res.status(404).send({ message: 'No data found to delete' })
    } catch (error) {
        res.status(500).send({ message: 'An error occurred when deleting data', error: error.message })
    } finally {
        await client.close()
    }
})