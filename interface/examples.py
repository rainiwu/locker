import lockpy


def loopback(outname="pyloopback.iq", freq=2.4e9, rxgain=20.0, txgain=10.0):
    '''
    Simultaneous TX/RX. Should output a continuous complex sinusoid.
    :param outname: name of output file
    :type outname: str
    :param freq: TX/RX frequency in hertz
    :type freq: float
    :param rxgain: rxgain in dB
    :type rxgain: float
    :param txgain: txgain in dB
    :type txgain: float
    '''
    assert isinstance(outname, str)
    assert isinstance(freq, float)
    assert isinstance(rxgain, float)
    assert isinstance(txgain, float)

    assert len(outname) > 0

    # initialize wrapper
    anInstance = lockpy.PyLock()

    # set arguments
    anInstance.rxfile = outname
    anInstance.freq = freq
    anInstance.rxgain = rxgain
    anInstance.txgain = txgain

    anInstance.rxrate = 2e6
    anInstance.txrate = 2e6

    # initialize instance
    anInstance.make_instance()

    # queue commands
    anInstance.queue_rx(2000000)
    anInstance.queue_tx(2000000)

    # execute commands
    anInstance.execute(0.1, 0.0)


def setup(addr0="", freq=2.4e9, rxrate=25e6, txrate=25e6, rxgain=20.0, txgain=10.0):
    assert isinstance(addr0, str)
    assert isinstance(freq, float)
    assert isinstance(rxrate, float)
    assert isinstance(txrate, float)
    assert isinstance(rxgain, float)
    assert isinstance(txgain, float)

    anInstance = lockpy.PyLock()
    # set arguments
    anInstance.freq = freq
    anInstance.rxgain = rxgain
    anInstance.txgain = txgain
    anInstance.rxrate = rxrate
    anInstance.txrate = txrate
    anInstance.addr0 = addr0

    return anInstance
