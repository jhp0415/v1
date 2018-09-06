# -*- coding : cp949 -*-
import socket
import tensorflow as tf
import matplotlib.pyplot as plt


#######################################################################################
def ReceiveData():
    #while True:
    data = conn.recv(1024)
    #if not data: break
    conn.send(data)  # 받은 데이터를 그대로 클라이언트에 전송
    data = data.decode()
    print(data[0], data[1], data[2])
    return data

#####################################################################################

def end_game(player_money, AI_money):
    print('플레이어 남은돈 : ', player_money, "\nAI_남은돈 : ", AI_money)
    print("****************************")
    '''
    if('A' == input('AI가 학습할 기회를 주고 싶다면 A 버튼을 입력하세요 : ')):
        return False
        '''
    print("AI가 학습할 기회를 주고 싶다면 A 버튼을 입력하세요 : ")

    return False
##########################################################################################
def learning():
    step_line = []
    cost_line = []

    print("\n학습중...")
    for step in range(10001):
        cost_val, _ = sess.run([cost, train], feed_dict={X: x_data, Y: y_data})
        step_line.append(step)
        cost_line.append(cost_val)
        if step % 2000 == 0:
            print(step, cost_val)
    print("학습완료.\n")
    plt.plot(step_line, cost_line)  # 리스트 변수를 매개변수로 던저 주면 그래프 그려줌
    plt.show()
#############################################################################################

X = tf.placeholder(tf.float32, shape = [None, 2]) #  X = [상대방도박평균,나의족보랭크]
Y = tf.placeholder(tf.float32, shape = [None, 1]) # win or lose

#모델링

#1차 모델링, l번 레이어
W1 = tf.Variable(tf.random_normal([2,4]), name = 'W1')
b1 = tf.Variable(tf.random_normal([4]), name = '1layer_bias')
layer1 = tf.sigmoid(tf.matmul(X,W1) + b1)

#2차 레이어
W3 = tf.Variable(tf.random_normal([4,1]), name = 'W3')
b3 = tf.Variable(tf.random_normal([1]), name = 'b3')
hypothesis = tf.sigmoid(tf.matmul(layer1, W3) + b3)

cost = -tf.reduce_mean(Y * tf.log(hypothesis) + (1 - Y) * tf.log(1 - hypothesis))
train = tf.train.GradientDescentOptimizer(learning_rate = 0.1).minimize(cost)

AI_money = 100.
player_money = 100.
x_data = []
x_temp = []
stake = 0
game_number = 1
CALL = 1.
y_data = []
AI_batting_record = []

sess = tf.Session()
sess.run(tf.global_variables_initializer())

############################소켓 서버############################################################
HOST = ''  # 호스트를 지정하지 않으면 가능한 모든 인터페이스를 의미한다.
PORT = 50007  # 포트지정
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)  # 접속이 있을때까지 기다림
print ('클라이언트 접속 대기중입니다..')
conn, addr = s.accept()  # 접속 승인
print('Connected by', addr)
##############################################################################################
conn.send(("start").encode())  # 클라이언트에 통신시작을 알림

while(True):
    game_continue = True

    #1게임, 게임플레이
    while(game_continue):
        print('\n기본배당금 일원으로 게임을 시작합니다. ')
        x_temp.clear()
        AI_money -= 1
        player_money -= 1
        stake = 2

        # x_temp = [족보랭크,도박]
        input_data = ReceiveData()

        # 족보랭크
        #AI_rank = float(input('AI의 족보를 입력하시오 : '))
        #player_rank = float(input('player의 족보를 입력하시오 : '))
        print('AI의 족보를 입력 : ', input_data[0])
        AI_rank = input_data[0]
        print('player의 족보를 입력 : ', input_data[1])
        player_rank = input_data[1]
        x_temp.append(AI_rank)
        #도박성
        #player_gambling = float(input('뭘 배팅할래? (0 : 다이  1 : 콜 )  : '))
        print('뭘 배팅할래? (0 : 다이  1 : 콜 )  :', input_data[2])
        player_gambling = float(input_data[2])
        x_temp.append(player_gambling * 10)

        x_data.append(list(x_temp))

        if(player_gambling == CALL):
            player_money -= stake / 2
            stake += stake / 2
            AI_gamble = sess.run(tf.cast(sess.run(hypothesis, feed_dict = {X : [x_data[-1]]}) > [[0.5]], dtype = tf.float32))
            AI_batting_record.append(AI_gamble)

            if(AI_batting_record[-1] == [CALL]):
                AI_money -= stake / 2
                stake += stake / 2
                print("****************************")
                print("AI가 CALL 했습니다.")

                if (AI_rank > player_rank):
                    print(game_number, "번째 경기결과\n", "AI가", AI_rank, '로 승리')
                    AI_money += stake
                    y_data.append([1.])
                    game_continue = end_game(player_money ,AI_money)
                    game_number += 1
                else:
                    print(game_number, "번째 경기결과\n", "player가", player_rank, '로 승리')
                    player_money += stake
                    y_data.append([0.])
                    game_continue = game_continue = end_game(player_money ,AI_money)
                    game_number += 1
            else:
                print("****************************")
                print("AI가 DIE 했습니다.")
                print(game_number,"번째 경기결과\nAI가 DIE 선언으로 player 승리")
                player_money += stake
                if(AI_rank > player_rank):
                    y_data.append([1.])
                else:
                    y_data.append([0.])

                game_continue = game_continue = end_game(player_money ,AI_money)
                game_number += 1
        else:
            print("****************************")
            print(game_number, "번째 경기결과\nplayer가 DIE 선언으로 AI 승리")
            AI_money += stake
            if (AI_rank > player_rank):
                y_data.append([1.])
            else:
                y_data.append([0.])
            game_continue = game_continue = end_game(player_money ,AI_money)
            game_number += 1

    learning()
    data = "ddd"
    conn.send(data.encode())  # 받은 데이터를 그대로 클라이언트에 전송


conn.close()










