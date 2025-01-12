import base64


def make_test(id):
    with open(f"blocks/seqno={id}.boc", "rb") as f:
        data = f.read()

    b64 = base64.b64encode(data).decode("utf-8")
    with open(f"tests-blocks/{id}.txt", "w") as f:
        f.write(f"compress\n{b64}\n")


def main():
    seqnos = range(43888281 - 100, 43888281)
    for x in seqnos:
        make_test(x)


if __name__ == "__main__":
    main()
