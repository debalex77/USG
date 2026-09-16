SELECT
    op.docPresentationDate AS title_order,
    tab.cod,
    tab.name AS Investigation,
    %price% AS Price
FROM
    orderEcho o
INNER JOIN
    orderEchoTable tab ON o.id = tab.id_orderEcho
INNER JOIN
    orderEchoPresentation op ON o.id = op.id_orderEcho
WHERE
    o.id = ? AND
    o.deletionMark = 2
ORDER BY
    tab.cod
