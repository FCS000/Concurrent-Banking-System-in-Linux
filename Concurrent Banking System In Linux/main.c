main.c CODE
#include "accounts.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
void execute_transaction(SharedData *s, int type, int source, int target, float amount, int
current_trans);
void print_results(SharedData *s);
int main() {
// 1. Paylaşılan bellek Shared memory
int shm_id = shmget(IPC_PRIVATE, sizeof(SharedData), 0666|IPC_CREAT);
if(shm_id == -1) {
perror("shmget failed");
exit(1);
}
SharedData *shared = (SharedData*)shmat(shm_id, NULL, 0);
if(shared == (void*)-1) {
perror("shmat failed");
exit(1);
}
// 2. Semafor init
sem_init(&shared->global_lock, 1, 1);
for(int i=0; i<MAX_ACCOUNTS; i++) {
sem_init(&shared->accounts[i].lock, 1, 1);
}
// 3. Hesapları başlat Start accounts
shared->transaction_count = 0;
for(int i=0; i<5; i++) {
    shared->accounts[i].account_id = i;
shared->accounts[i].balance = 1000.0;
}
// 4. Dosyayı aç Open file
FILE *file = fopen("transactions.txt", "r");
if(!file) {
perror("Failed to open file");
exit(1);
}
// 5. İşlemleri oku ve işle Read and process transactions
int type, source, target;
float amount;
char line[100];
while(fgets(line, sizeof(line), file)) {
if(sscanf(line, "%d %d %d %f", &type, &source, &target, &amount) != 4) {
fprintf(stderr, "Invalid line format: %s", line);
continue;
}
printf("Processing: type=%d from=%d to=%d amount=%.2f\n",
type, source, target, amount);
sem_wait(&shared->global_lock);
if(shared->transaction_count >= MAX_TRANSACTIONS) {
sem_post(&shared->global_lock);
fprintf(stderr, "Transaction limit reached (%d)\n", MAX_TRANSACTIONS);
continue;
}
int current_trans = shared->transaction_count++;
sem_post(&shared->global_lock);
pid_t pid = fork();
if(pid == 0) { // Child
execute_transaction(shared, type, source, target, amount, current_trans);
_exit(0);
}
else if(pid > 0) { // Parent
int status;
waitpid(pid, &status, 0);
}
else {
perror("fork failed");
}
}
fclose(file);
// 6. Sonuçları yazdır Print results
print_results(shared);
// 7. Temizlik Cleaning
shmdt(shared);
shmctl(shm_id, IPC_RMID, NULL);
return 0;
}
void execute_transaction(SharedData *s, int type, int source, int target, float amount, int
current_trans) {
// Kaydı doldur Fill out registration
s->transactions[current_trans].transaction_id = current_trans;
s->transactions[current_trans].type = type;
s->transactions[current_trans].source_account = source;
s->transactions[current_trans].target_account = target;
s->transactions[current_trans].amount = amount;
s->transactions[current_trans].timestamp = time(NULL);
switch(type) {
case 0: // Para çekme Withdrawal ( NEW 1.1 FİX)
sem_wait(&s->accounts[source].lock);
if(s->accounts[source].balance >= amount) {
s->accounts[source].balance -= amount;
s->transactions[current_trans].status = 0;
printf("Success: Withdrew %.2f from %d\n", amount, source);
} if(s->accounts[source].balance <= amount){
s->transactions[current_trans].status = -1;
printf("Failed: Insufficient balance in %d (Current: %.2f)\n",
source, s->accounts[source].balance);
}
sem_post(&s->accounts[source].lock);
break;
case 1: // Para yatırma Deposit money
sem_wait(&s->accounts[target].lock);
s->accounts[target].balance += amount;
s->transactions[current_trans].status = 0;
printf("Success: Deposited %.2f to %d\n", amount, target);
sem_post(&s->accounts[target].lock);
break;

case 2: // Transfer
int first = (source < target) ? source : target;
int second = (source < target) ? target : source;
sem_wait(&s->accounts[first].lock);
sem_wait(&s->accounts[second].lock);
if(s->accounts[source].balance >= amount) {
s->accounts[source].balance -= amount;
s->accounts[target].balance += amount;
s->transactions[current_trans].status = 0;
printf("Success: Transferred %.2f from %d to %d\n", amount, source, target);
} else {
s->transactions[current_trans].status = -1;
printf("Failed: Transfer from %d to %d\n", source, target);
}
sem_post(&s->accounts[second].lock);
sem_post(&s->accounts[first].lock);
break;
}
}
void print_results(SharedData *s) {
printf("\nFINAL ACCOUNT BALANCES:\n");
for(int i=0; i<5; i++) {
printf("Account %d: %.2f\n", i, s->accounts[i].balance);
}
printf("\nTRANSACTION LOG:\n");
printf("ID | Type | From | To | Amount | Status | Timestamp\n");
printf("----------------------------------------------------------\n");
for(int i=0; i<s->transaction_count; i++) {
Transaction t = s->transactions[i];
char time_str[26];
ctime_r(&t.timestamp, time_str);
time_str[strlen(time_str)-1] = '\0'; // Remove newline
const char* type_str;
switch(t.type) {
case 0: type_str = "WITHDRAW"; break;
case 1: type_str = "DEPOSIT "; break;
case 2: type_str = "TRANSFER"; break;
default: type_str = "UNKNOWN"; break;
}

printf("%2d | %-8s | %4d | %3d | %7.2f | %-8s | %s\n",
t.transaction_id,
type_str,
t.source_account,
t.target_account,
t.amount,
t.status == 0 ? "SUCCESS" : "FAILED",
time_str);
}
}
