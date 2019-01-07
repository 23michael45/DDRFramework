import BaseCmd_pb2 as PB


def Move():
    reqCmdMove = PB.reqCmdMove()
    reqCmdMove.line_speed = 111
    reqCmdMove.angulau_speed = 222
    s = address_book.SerializeToString()

    print(s)
    return s
