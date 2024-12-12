# Simulating a bank

This project simulates the banking by reading in customer actions, processing transactions given bank policies, responding to queries, and analyzing transactions to detect fraudulent behavior. 
Here's the basic functionalities that this solution covers:
1. Users are able to perform 5 operations: Login, Logout, Check their balance, or put a transaction in place for now or up to 3 days in the future.
2. Each transaction is completed only when assuring that the sender and receiver are valid accounts with the bank at the time of transaction, if it is not a self-transaction, and the sender is logged in and has sufficient funds. It also levies a transaction fees
3. The following queries can be made: List all transactions between two timestamps, calculate the bank's revenue between two timestamps, get a customer's history, and get a summary of all activities from a given day.

